#include "cscoin-solver.h"
#include "cscoin-mt64.h"

#include <omp.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>

static gint
gint64cmp_asc (const void *a, const void *b)
{
    return *(guint64*) a < *(guint64*) b ? -1 : 1;
}

static gint
gint64cmp_desc (const void *a, const void *b)
{
    return *(guint64*) a > *(guint64*) b ? -1 : 1;
}

enum CSCoinChallengeType
{
    CSCOIN_CHALLENGE_SORTED_LIST,
    CSCOIN_CHALLENGE_REVERSE_SORTED_LIST
};

gchar *
cscoin_solve_challenge (gint           challenge_id,
                        const gchar   *challenge_name,
                        const gchar   *last_solution_hash,
                        const gchar   *hash_prefix,
                        gint           nb_elements,
                        GCancellable  *cancellable,
                        GError       **error)
{
    enum CSCoinChallengeType challenge_type;
    gboolean done = FALSE;
    gchar *ret = NULL;
    guint16 hash_prefix_num;

    if (strcmp (challenge_name, "sorted_list") == 0)
    {
        challenge_type = CSCOIN_CHALLENGE_SORTED_LIST;
    }
    else if (strcmp (challenge_name, "reverse_sorted_list") == 0)
    {
        challenge_type = CSCOIN_CHALLENGE_REVERSE_SORTED_LIST;
    }
    else
    {
        g_return_val_if_reached (NULL);
    }

    hash_prefix_num = GUINT16_FROM_BE (strtol (hash_prefix, NULL, 16));

    #pragma omp parallel
    {
        SHA256_CTX checksum;
        CSCoinMT64 mt64;
        union {
            guint8  digest[SHA256_DIGEST_LENGTH];
            guint64 seed;
            guint16 prefix;
        } checksum_digest;
        guint64 numbers[nb_elements];
        guint nonce;
        gchar nonce_str[32];
        gint i;

        cscoin_mt64_init (&mt64);

        /* OpenMP partitionning */
        guint nonce_from, nonce_to;
        guint nonce_partition_size = G_MAXUINT / omp_get_num_threads ();
        nonce_from = omp_get_thread_num () * nonce_partition_size;
        nonce_to   = nonce_from + nonce_partition_size;

        for (nonce = nonce_from; nonce < nonce_to; nonce++)
        {
            if (G_UNLIKELY (done || g_cancellable_is_cancelled (cancellable)))
            {
                break;
            }

            g_snprintf (nonce_str, 32, "%u", nonce);

            SHA256_Init (&checksum);
            SHA256_Update (&checksum, last_solution_hash, 64);
            SHA256_Update (&checksum, nonce_str, strlen (nonce_str));
            SHA256_Final (checksum_digest.digest, &checksum);

            cscoin_mt64_set_seed (&mt64, GUINT64_FROM_LE (checksum_digest.seed));

            for (i = 0; i < nb_elements; i++)
            {
                numbers[i] = cscoin_mt64_next_uint64 (&mt64);
            }

            switch (challenge_type)
            {
                case CSCOIN_CHALLENGE_SORTED_LIST:
                    qsort (numbers, nb_elements, sizeof (guint64), gint64cmp_asc);
                    break;
                case CSCOIN_CHALLENGE_REVERSE_SORTED_LIST:
                    qsort (numbers, nb_elements, sizeof (guint64), gint64cmp_desc);
                    break;
            }

            SHA256_Init (&checksum);

            gchar number_str[32];
            for (i = 0; i < nb_elements; i++)
            {
                g_snprintf (number_str, 32, "%lu", numbers[i]);
                SHA256_Update (&checksum, number_str, strlen (number_str));
            }

            SHA256_Final (checksum_digest.digest, &checksum);

            if (hash_prefix_num == GUINT16_FROM_LE (checksum_digest.prefix))
            {
                done = TRUE;
                ret = g_strdup (nonce_str);
            }
        }
    }

    if (g_cancellable_set_error_if_cancelled (cancellable, error))
    {
        return NULL;
    }

    return ret;
}
