#include "cscoin-solver.h"
#include "cscoin-mt64.h"

#include <omp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <astar.h>

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

typedef enum _CSCoinShortestPathTileType CSCoinShortestPathTileType;

enum _CSCoinShortestPathTileType
{
    BLANK   = 0,
    START   = 1,
    END     = 2,
    BLOCKER = 3
};

typedef struct _CSCoinShortestPathGrid CSCoinShortestPathGrid;

struct _CSCoinShortestPathGrid
{
    CSCoinShortestPathTileType *tiles;
    gint                        size;
};

static guint8
get_grid_cost (const guint32 x, const guint32 y, void* user_data)
{
    CSCoinShortestPathGrid *grid = user_data;
    return grid->tiles[y * grid->size + x] == BLOCKER ? COST_BLOCKED : 1;
}

static void
solve_shortest_path_challenge (CSCoinMT64 *mt64,
                               SHA256_CTX *checksum,
                               gint        grid_size,
                               gint        nb_blockers)
{
    CSCoinShortestPathTileType grid[grid_size][grid_size];
    guint64 x0, y0, x1, y1;
    gint i, j;

    // initialize with blanks which is '0'
    memset (grid, 0, sizeof (grid));

    // Borders
    for (i = 0; i < grid_size; i++)
    {
        grid[0][i]             = BLOCKER;
        grid[grid_size - 1][i] = BLOCKER;
    }

    for (j = 1; i < grid_size - 1; j++)
    {
        grid[j][0]             = BLOCKER;
        grid[j][grid_size - 1] = BLOCKER;
    }

    // Start/End
    for(;;)
    {
        y0 = cscoin_mt64_next_uint64 (mt64) % grid_size;
        x0 = cscoin_mt64_next_uint64 (mt64) % grid_size;

        if (grid[y0][x0] == BLANK)
        {
            grid[y0][x0] = START;
            break;
        }
    }

    for(;;)
    {
        y1 = cscoin_mt64_next_uint64 (mt64) % grid_size;
        x1 = cscoin_mt64_next_uint64 (mt64) % grid_size;

        if (grid[y1][x1] == BLANK)
        {
            grid[y1][x1] = END;
            break;
        }
    }

    // Blockers
    for (i = 0; i < nb_blockers; i++)
    {
        guint64 row = cscoin_mt64_next_uint64 (mt64) % grid_size;
        guint64 col = cscoin_mt64_next_uint64 (mt64) % grid_size;

        if (grid[row][col] == BLANK)
        {
            grid[row][col] = BLOCKER;
        }
    }

    // Debugging
    /*
    for (i = 0; i < grid_size; i++)
    {
        for (j = 0; j < grid_size; j++)
        {
            g_printf("%i", grid[j][i]);
        }
        g_printf("\n");
    }
    g_printf("\n");
    */

    astar_t * as;

    CSCoinShortestPathGrid user_data = { .tiles = grid, .size = grid_size };
    as = astar_new (grid_size, grid_size, get_grid_cost, &user_data, NULL);

    astar_set_origin (as, 0, 0);
    astar_set_movement_mode (as, DIR_CARDINAL);

    // TODO Set cost
    /*
    astar_set_cost (as, DIR_NE, 100);
    astar_set_cost (as, DIR_NW, 100);
    astar_set_cost (as, DIR_SW, 100);
    astar_set_cost (as, DIR_SE, 100);
    */

    gint result = astar_run (as, x0, y0, x1, y1);

    // Debugging
    g_printf ("Route from (%d, %d) to (%d, %d). Result: %s (%d)\n",
            as->x0, as->y0,
            as->x1, as->y1,
            as->str_result, result);

    astar_destroy (as);
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
