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

static const GEnumValue
CHALLENGE_TYPE_ENUM_VALUES[] =
{
    {0, "CSCOIN_CHALLENGE_TYPE_SORTED_LIST",         "sorted_list"},
    {1, "CSCOIN_CHALLENGE_TYPE_REVERSE_SORTED_LIST", "reverse_sorted_list"},
    {2, "CSCOIN_CHALLENGE_TYPE_SHORTEST_PATH",       "shortest_path"},
    NULL
};

static GType
cscoin_challenge_type = 0;

GType
cscoin_challenge_type_get_type ()
{
    if (g_once_init_enter (&cscoin_challenge_type))
    {
        GType cscoin_challenge_type_value = g_enum_register_static ("CSCoinChallengeType", CHALLENGE_TYPE_ENUM_VALUES);
        g_once_init_leave (&cscoin_challenge_type, cscoin_challenge_type_value);
    }

    return cscoin_challenge_type;
}

G_DEFINE_BOXED_TYPE (CSCoinChallengeParameters,
                     cscoin_challenge_parameters,
                     cscoin_challenge_parameters_copy,
                     cscoin_challenge_parameters_free);

gpointer
cscoin_challenge_parameters_copy (gpointer boxed)
{
    CSCoinChallengeParameters *ret = g_malloc (sizeof (CSCoinChallengeParameters));

    memcpy (ret, boxed, sizeof (CSCoinChallengeParameters));

    return ret;
}

void
cscoin_challenge_parameters_free (gpointer boxed)
{
    g_free (boxed);
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
        guint nonce;
        gchar nonce_str[32];

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
                    break;
                default:
                    /* cannot break from OpenMP section */
                    g_critical ("Unknown challenge type %d, nothing will be performed.", challenge_type);
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
