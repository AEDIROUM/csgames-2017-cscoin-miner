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

typedef gboolean (*CSCoinChallengeSolverFunc) (CSCoinMT64                *mt64,
                                               SHA256_CTX                *checksum,
                                               CSCoinChallengeParameters *parameters);

static gboolean
solve_sorted_list_challenge (CSCoinMT64 *mt64,
                             SHA256_CTX *checksum,
                             CSCoinChallengeParameters *parameters)
{
    gint i;
    gint nb_elements = parameters->sorted_list.nb_elements;
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

    return TRUE;
}

static gboolean
solve_reverse_sorted_list_challenge (CSCoinMT64                *mt64,
                                     SHA256_CTX                *checksum,
                                     CSCoinChallengeParameters *parameters)
{
    gint i;
    gint nb_elements = parameters->reverse_sorted_list.nb_elements;
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

    return TRUE;
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

static gboolean
solve_shortest_path_challenge (CSCoinMT64 *mt64,
                               SHA256_CTX *checksum,
                               CSCoinChallengeParameters *parameters)
{
    gint grid_size   = parameters->shortest_path.grid_size;
    gint nb_blockers = parameters->shortest_path.nb_blockers;
    CSCoinShortestPathTileType grid[grid_size][grid_size];
    guint64 x0, y0, x1, y1;
    guint64 x, y;
    gint i, j;

    // initialize with blanks which is '0'
    memset (grid, BLANK, sizeof (grid));

    // X : COL Y : ROW
    // grid[y][x]
    // Borders
    for (i = 0; i < grid_size; i++)
    {
        grid[0][i]             = BLOCKER;
        grid[grid_size - 1][i] = BLOCKER;
    }

    for (j = 1; j < grid_size - 1; j++)
    {
        grid[j][0]             = BLOCKER;
        grid[j][grid_size - 1] = BLOCKER;
    }

    // Start
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

    // End
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
        y = cscoin_mt64_next_uint64 (mt64) % grid_size;
        x = cscoin_mt64_next_uint64 (mt64) % grid_size;

        if (grid[y][x] == BLANK)
        {
            grid[y][x] = BLOCKER;
        }
    }

    // Debugging
    g_printf("grid:\n");
    for (j = 0; j < grid_size; j++)
    {
        for (i = 0; i < grid_size; i++)
        {
            g_printf("%u ", grid[j][i]);
        }
        g_printf("\n");
    }

    astar_t as;

    CSCoinShortestPathGrid user_data = { .tiles = (CSCoinShortestPathTileType*) grid, .size = grid_size };
    astar_init (&as, grid_size, grid_size, get_grid_cost, &user_data, NULL);

    astar_set_origin (&as, 0, 0);
    astar_set_movement_mode (&as, DIR_CARDINAL);

    /*
    astar_set_cost (&as, DIR_N, 10);
    astar_set_cost (&as, DIR_W, 20);
    astar_set_cost (&as, DIR_E, 30);
    astar_set_cost (&as, DIR_S, 40);
    */
    /*
    astar_set_steering_penalty (&as, 10);
    astar_set_heuristic_factor (&as, 10);
    */
    // astar_set_max_cost (100);

    gint result = astar_run (&as, x0, y0, x1, y1);

    if (result == ASTAR_FOUND && astar_have_route (&as))
    {
        guint32 num_steps;
        direction_t *directions;
        gchar number_str[32];

        num_steps = astar_get_directions (&as, &directions);

        x = x0;
        y = y0;

        /* entry */
        g_snprintf (number_str, 32, "%lu", y);
        SHA256_Update (checksum, number_str, strlen (number_str));
        g_snprintf (number_str, 32, "%lu", x);
        SHA256_Update (checksum, number_str, strlen (number_str));

        /* hash all other coordinates including the exit */
        for (i = 0; i < num_steps; i++)
        {
            x += astar_get_dx (&as, directions[i]);
            y += astar_get_dy (&as, directions[i]);
            g_printf ("(%lu, %lu)", y, x);
            g_snprintf (number_str, 32, "%lu", y);
            SHA256_Update (checksum, number_str, strlen (number_str));
            g_snprintf (number_str, 32, "%lu", x);
            SHA256_Update (checksum, number_str, strlen (number_str));
        }

        astar_free_directions (directions);
        astar_clear (&as);

        // Debugging
        //g_printf ("Route from (%d, %d) to (%d, %d). Result: %s (%d)\n",
        //        as.x0, as.y0,
        //        as.x1, as.y1,
        //        as.str_result, result);
        return TRUE;
    }
    else
    {
        astar_clear (&as);
        return FALSE;
    }
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
    CSCoinChallengeSolverFunc solver_func;

    hash_prefix_num = GUINT16_FROM_BE (strtol (hash_prefix, NULL, 16));

    switch (challenge_type)
    {
        case CSCOIN_CHALLENGE_TYPE_SORTED_LIST:
            solver_func = solve_sorted_list_challenge;
            break;
        case CSCOIN_CHALLENGE_TYPE_REVERSE_SORTED_LIST:
            solver_func = solve_reverse_sorted_list_challenge;
            break;
        case CSCOIN_CHALLENGE_TYPE_SHORTEST_PATH:
            solver_func = solve_shortest_path_challenge;
            break;
        default:
            g_return_val_if_reached (NULL);
    }

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

            if (solver_func (&mt64, &checksum, parameters))
            {
                SHA256_Final (checksum_digest.digest, &checksum);

                if (hash_prefix_num == GUINT16_FROM_LE (checksum_digest.prefix))
                {
                    done = TRUE;
                    ret = g_strdup (nonce_str);
                }
            }
        }
    }

    if (g_cancellable_set_error_if_cancelled (cancellable, error))
    {
        return NULL;
    }

    return ret;
}
