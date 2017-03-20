#include "cscoin-solver.h"
#include "cscoin-mt64.h"

#include <omp.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>

static gint
guint64cmp_asc (const void *a, const void *b)
{
    return *(guint64*) a < *(guint64*) b ? -1 : 1;
}

static gint
guint64cmp_desc (const void *a, const void *b)
{
    return *(guint64*) a > *(guint64*) b ? -1 : 1;
}

static void
solve_sorted_list_challenge (CSCoinMT64 *mt64,
                             SHA256_CTX *checksum,
                             gint        nb_elements)
{
    gint i;
    guint64 numbers[nb_elements];

    for (i = 0; i < nb_elements; i++)
    {
        numbers[i] = cscoin_mt64_next_uint64 (mt64);
    }

    qsort (numbers, nb_elements, sizeof (guint64), guint64cmp_asc);

    gchar number_str[32];
    for (i = 0; i < nb_elements; i++)
    {
        g_snprintf (number_str, 32, "%lu", numbers[i]);
        SHA256_Update (checksum, number_str, strlen (number_str));
    }
}

static void
solve_reverse_sorted_list_challenge (CSCoinMT64 *mt64,
                                     SHA256_CTX *checksum,
                                     gint        nb_elements)
{
    gint i;
    guint64 numbers[nb_elements];

    for (i = 0; i < nb_elements; i++)
    {
        numbers[i] = cscoin_mt64_next_uint64 (mt64);
    }

    qsort (numbers, nb_elements, sizeof (guint64), guint64cmp_desc);

    gchar number_str[32];
    for (i = 0; i < nb_elements; i++)
    {
        g_snprintf (number_str, 32, "%lu", numbers[i]);
        SHA256_Update (checksum, number_str, strlen (number_str));
    }
}

static void
solve_shortest_path_challenge (CSCoinMT64 *mt64,
                               SHA256_CTX *checksum,
                               gint        grid_size,
                               gint        nb_blockers)
{
    // TODO
}

gchar *
cscoin_solve_challenge (gint                        challenge_id,
                        CSCoinChallengeType         challenge_type,
                        const gchar                *last_solution_hash,
                        const gchar                *hash_prefix,
                        CSCoinChallengeParameters  *parameters,
                        GCancellable               *cancellable,
                        GError                    **error)
{
    gboolean done = FALSE;
    gchar *ret = NULL;
    guint16 hash_prefix_num;

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
        guint64 nonce;
        gchar nonce_str[32];
        GEnumClass *challenge_type_enum_class;

        cscoin_mt64_init (&mt64);

        /* OpenMP partitionning */
        guint64 nonce_from, nonce_to;
        guint64 nonce_partition_size = G_MAXUINT64 / omp_get_num_threads ();
        nonce_from = omp_get_thread_num () * nonce_partition_size;
        nonce_to   = nonce_from + nonce_partition_size;

        for (nonce = nonce_from; nonce < nonce_to; nonce++)
        {
            if (G_UNLIKELY (done || g_cancellable_is_cancelled (cancellable)))
            {
                break;
            }

            g_snprintf (nonce_str, 32, "%lu", nonce);

            SHA256_Init (&checksum);
            SHA256_Update (&checksum, last_solution_hash, 64);
            SHA256_Update (&checksum, nonce_str, strlen (nonce_str));
            SHA256_Final (checksum_digest.digest, &checksum);

            cscoin_mt64_set_seed (&mt64, GUINT64_FROM_LE (checksum_digest.seed));

            SHA256_Init (&checksum);

            switch (challenge_type)
            {
                case CSCOIN_CHALLENGE_TYPE_SORTED_LIST:
                    solve_sorted_list_challenge (&mt64,
                                                 &checksum,
                                                 parameters->sorted_list.nb_elements);
                    break;
                case CSCOIN_CHALLENGE_TYPE_REVERSE_SORTED_LIST:
                    solve_reverse_sorted_list_challenge (&mt64,
                                                         &checksum,
                                                         parameters->reverse_sorted_list.nb_elements);
                    break;
                case CSCOIN_CHALLENGE_TYPE_SHORTEST_PATH:
                    solve_shortest_path_challenge (&mt64,
                                                   &checksum,
                                                   parameters->shortest_path.grid_size,
                                                   parameters->shortest_path.nb_blockers);
                    // break;
                default:
                    /* cannot break from OpenMP section */
                    challenge_type_enum_class = g_type_class_peek (CSCOIN_TYPE_CHALLENGE_TYPE);
                    g_critical ("Unknown challenge type '%s', nothing will be performed.",
                                g_enum_get_value (challenge_type_enum_class, challenge_type)->value_name);
                    done = TRUE;
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
