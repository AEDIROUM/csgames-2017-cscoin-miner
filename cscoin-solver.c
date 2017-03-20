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

typedef enum _CSCoinShortestPathTileType CSCoinShortestPathTileType;

enum _CSCoinShortestPathTileType
{
    CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK    = 0b00,
    CSCOIN_SHORTEST_PATH_TILE_TYPE_ENTRY    = 0b01,
    CSCOIN_SHORTEST_PATH_TILE_TYPE_EXIT     = 0b10,
    CSCOIN_SHORTEST_PATH_TILE_TYPE_FRONTIER = 0b11
};

typedef enum _CSCoinShortestPathPosition CSCoinShortestPathPosition;

enum _CSCoinShortestPathPosition
{
    CSCOIN_SHORTEST_PATH_POSITION_UP_LEFT    = 0,
    CSCOIN_SHORTEST_PATH_POSITION_UP_RIGHT   = 1,
    CSCOIN_SHORTEST_PATH_POSITION_DOWN_LEFT  = 2,
    CSCOIN_SHORTEST_PATH_POSITION_DOWN_RIGHT = 3
};

/**
 * Indicate the cost reaching exit on the tile when entering in a position.
 *
 * The tiles are indexed such that each 2-bit pack represent a tile
 *
 *  - 00 is a blank
 *  - 01 is an entry
 *  - 10 is an exit
 *  - 11 is a frontier
 *
 * The position are enumerated from up-left, up-right, down-left to down-right.
 *
 * If there is no exit, then the cost is '0', which indicates that the
 * tile should be skipped.
 *
 * Only entries with exactly one exit (i.e. '10') and at most one entry (i.e '01')
 * are filled.
 */
static guint8
CSCOIN_SHORTEST_PATH_TILE_COST_PER_POSITION[256][4] =
{
    {0, 0, 0, 0}, // 0b00_00_00_00
    {0, 0, 0, 0}, // 0b00_00_00_01
    {3, 2, 2, 1}, // 0b00_00_00_10
    {0, 0, 0, 0}, // 0b00_00_00_11
    {0, 0, 0, 0}, // 0b00_00_01_00
    {0, 0, 0, 0}, // 0b00_00_01_01
    {3, 2, 2, 1}, // 0b00_00_01_10
    {0, 0, 0, 0}, // 0b00_00_01_11
    {0, 0, 0, 0}, // 0b00_01_00_00
    {0, 0, 0, 0}, // 0b00_01_00_01
    {3, 2, 2, 1}, // 0b00_01_00_10
    {0, 0, 0, 0}, // 0b00_01_00_11
    {0, 0, 0, 0}, // 0b00_01_01_00
    {0, 0, 0, 0}, // 0b00_01_01_01
    {0, 0, 0, 0}, // 0b00_01_01_10
    {0, 0, 0, 0}, // 0b00_01_01_11
    {2, 1, 3, 2}, // 0b00_10_00_00
    {2, 1, 3, 2}, // 0b00_10_00_01
    {0, 0, 0, 0}, // 0b00_10_00_10
    {2, 1, 3, 2}, // 0b00_10_00_11
    {2, 1, 3, 2}, // 0b00_10_01_00
    {0, 0, 0, 0}, // 0b00_10_01_01
    {0, 0, 0, 0}, // 0b00_10_01_10
    {0, 0, 0, 0}, // 0b00_11_01_11
    {0, 0, 0, 0}, // 0b00_11_00_00
    {0, 0, 0, 0}, // 0b00_11_00_01
    {3, 2, 2, 1}, // 0b00_11_00_10
    {0, 0, 0, 0}, // 0b00_11_00_11
    {0, 0, 0, 0}, // 0b00_11_01_00
    {0, 0, 0, 0}, // 0b00_11_01_01
    {3, 2, 2, 1}, // 0b00_11_01_10
    {0, 0, 0, 0}, // 0b00_11_01_11
    {0, 0, 0, 0}, // 0b01_00_00_00
    {0, 0, 0, 0}, // 0b01_00_00_01
    {3, 2, 2, 1}, // 0b01_00_00_10
    {0, 0, 0, 0}, // 0b01_00_00_11
    {0, 0, 0, 0}, // 0b01_00_01_00
    {0, 0, 0, 0}, // 0b01_00_01_01
    {0, 0, 0, 0}, // 0b01_00_01_10
    {0, 0, 0, 0}, // 0b01_00_01_11
    {0, 0, 0, 0}, // 0b01_01_00_00
    {0, 0, 0, 0}, // 0b01_01_00_01
    {0, 0, 0, 0}, // 0b01_01_00_10
    {0, 0, 0, 0}, // 0b01_01_00_11
    {0, 0, 0, 0}, // 0b01_01_01_00
    {0, 0, 0, 0}, // 0b01_01_01_01
    {0, 0, 0, 0}, // 0b01_01_01_10
    {0, 0, 0, 0}, // 0b01_01_01_11
    {2, 1, 3, 2}, // 0b01_10_00_00
    {0, 0, 0, 0}, // 0b01_10_00_01
    {0, 0, 0, 0}, // 0b01_10_00_10
    {2, 1, 3, 2}, // 0b01_10_00_11
    {0, 0, 0, 0}, // 0b01_10_01_00
    {0, 0, 0, 0}, // 0b01_10_01_01
    {0, 0, 0, 0}, // 0b01_10_01_10
    {0, 0, 0, 0}, // 0b01_10_01_11
    {0, 0, 0, 0}, // 0b01_11_00_00
    {0, 0, 0, 0}, // 0b01_11_00_01
    {3, 2, 2, 1}, // 0b01_11_00_10
    {0, 0, 0, 0}, // 0b01_11_00_11
    {0, 0, 0, 0}, // 0b01_11_01_00
    {0, 0, 0, 0}, // 0b01_11_01_01
    {0, 0, 0, 0}, // 0b01_11_01_10
    {0, 0, 0, 0}, // 0b01_11_01_11
    {0, 0, 0, 0}, // 0b00_00_00_00
    {0, 0, 0, 0}, // 0b00_00_00_01
    {3, 2, 2, 1}, // 0b00_00_00_10
    {0, 0, 0, 0}, // 0b00_00_00_11
    {0, 0, 0, 0}, // 0b00_00_01_00
    {0, 0, 0, 0}, // 0b00_00_01_01
    {3, 2, 2, 1}, // 0b00_00_01_10
    {0, 0, 0, 0}, // 0b00_00_01_11
    {0, 0, 0, 0}, // 0b00_01_00_00
    {0, 0, 0, 0}, // 0b00_01_00_01
    {3, 2, 2, 1}, // 0b00_01_00_10
    {0, 0, 0, 0}, // 0b00_01_00_11
    {0, 0, 0, 0}, // 0b00_01_01_00
    {0, 0, 0, 0}, // 0b00_01_01_01
    {0, 0, 0, 0}, // 0b00_01_01_10
    {0, 0, 0, 0}, // 0b00_01_01_11
    {2, 1, 3, 2}, // 0b00_10_00_00
    {2, 1, 3, 2}, // 0b00_10_00_01
    {0, 0, 0, 0}, // 0b00_10_00_10
    {2, 1, 3, 2}, // 0b00_10_00_11
    {2, 1, 3, 2}, // 0b00_10_01_00
    {0, 0, 0, 0}, // 0b00_10_01_01
    {0, 0, 0, 0}, // 0b00_10_01_10
    {2, 1, 3, 2}, // 0b00_10_01_11
    {0, 0, 0, 0}, // 0b00_11_00_00
    {0, 0, 0, 0}, // 0b00_11_00_01
    {3, 2, 2, 1}, // 0b00_11_00_10
    {0, 0, 0, 0}, // 0b00_11_00_11
    {0, 0, 0, 0}, // 0b00_11_01_00
    {0, 0, 0, 0}, // 0b00_11_01_01
    {3, 2, 2, 1}, // 0b00_11_01_10
    {0, 0, 0, 0}, // 0b00_11_01_11
    {0, 0, 0, 0}, // 0b01_00_00_00
    {0, 0, 0, 0}, // 0b01_00_00_01
    {3, 2, 2, 1}, // 0b01_00_00_10
    {0, 0, 0, 0}, // 0b01_00_00_11
    {0, 0, 0, 0}, // 0b01_00_01_00
    {0, 0, 0, 0}, // 0b01_00_01_01
    {0, 0, 0, 0}, // 0b01_00_01_10
    {0, 0, 0, 0}, // 0b01_00_01_11
    {0, 0, 0, 0}, // 0b01_01_00_00
    {0, 0, 0, 0}, // 0b01_01_00_01
    {0, 0, 0, 0}, // 0b01_01_00_10
    {0, 0, 0, 0}, // 0b01_01_00_11
    {0, 0, 0, 0}, // 0b01_01_01_00
    {0, 0, 0, 0}, // 0b01_01_01_01
    {0, 0, 0, 0}, // 0b01_01_01_10
    {0, 0, 0, 0}, // 0b01_01_01_11
    {2, 1, 3, 2}, // 0b01_10_00_00
    {0, 0, 0, 0}, // 0b01_10_00_01
    {0, 0, 0, 0}, // 0b01_10_00_10
    {2, 1, 3, 2}, // 0b01_10_00_11
    {0, 0, 0, 0}, // 0b01_10_01_00
    {0, 0, 0, 0}, // 0b01_10_01_01
    {0, 0, 0, 0}, // 0b01_10_01_10
    {0, 0, 0, 0}, // 0b01_10_01_11
    {0, 0, 0, 0}, // 0b01_11_00_00
    {0, 0, 0, 0}, // 0b01_11_00_01
    {3, 2, 2, 1}, // 0b01_11_00_10
    {0, 0, 0, 0}, // 0b01_11_00_11
    {0, 0, 0, 0}, // 0b01_11_01_00
    {0, 0, 0, 0}, // 0b01_11_01_01
    {0, 0, 0, 0}, // 0b01_11_01_10
    {0, 0, 0, 0}, // 0b01_11_01_11
    {1, 2, 2, 3}, // 0b10_00_00_00
    {1, 2, 2, 3}, // 0b10_00_00_01
    {0, 0, 0, 0}, // 0b10_00_00_10
    {1, 2, 2, 3}, // 0b10_00_00_11
    {1, 2, 2, 3}, // 0b10_00_01_00
    {0, 0, 0, 0}, // 0b10_00_01_01
    {0, 0, 0, 0}, // 0b10_00_01_10
    {1, 2, 2, 3}, // 0b10_00_01_11
    {1, 2, 2, 3}, // 0b10_01_00_00
    {0, 0, 0, 0}, // 0b10_01_00_01
    {0, 0, 0, 0}, // 0b10_01_00_10
    {1, 2, 2, 3}, // 0b10_01_00_11
    {0, 0, 0, 0}, // 0b10_01_01_00
    {0, 0, 0, 0}, // 0b10_01_01_01
    {0, 0, 0, 0}, // 0b10_01_01_10
    {0, 0, 0, 0}, // 0b10_01_01_11
    {0, 0, 0, 0}, // 0b10_10_00_00
    {0, 0, 0, 0}, // 0b10_10_00_01
    {0, 0, 0, 0}, // 0b10_10_00_10
    {0, 0, 0, 0}, // 0b10_10_00_11
    {0, 0, 0, 0}, // 0b10_10_01_00
    {0, 0, 0, 0}, // 0b10_10_01_01
    {0, 0, 0, 0}, // 0b10_10_01_10
    {0, 0, 0, 0}, // 0b10_10_01_11
    {1, 2, 2, 3}, // 0b10_11_00_00
    {1, 2, 2, 3}, // 0b10_11_00_01
    {0, 0, 0, 0}, // 0b10_11_00_10
    {1, 2, 2, 3}, // 0b10_11_00_11
    {1, 2, 2, 3}, // 0b10_11_01_00
    {0, 0, 0, 0}, // 0b10_11_01_01
    {0, 0, 0, 0}, // 0b10_11_01_10
    {1, 2, 2, 3}, // 0b10_11_01_11
    {0, 0, 0, 0}, // 0b11_00_00_00
    {0, 0, 0, 0}, // 0b11_00_00_01
    {3, 2, 2, 1}, // 0b11_00_00_10
    {0, 0, 0, 0}, // 0b11_00_00_11
    {0, 0, 0, 0}, // 0b11_00_01_00
    {0, 0, 0, 0}, // 0b11_00_01_01
    {3, 2, 2, 1}, // 0b11_00_01_10
    {0, 0, 0, 0}, // 0b11_00_01_11
    {0, 0, 0, 0}, // 0b11_01_00_00
    {0, 0, 0, 0}, // 0b11_01_00_01
    {3, 2, 2, 1}, // 0b11_01_00_10
    {0, 0, 0, 0}, // 0b11_01_00_11
    {0, 0, 0, 0}, // 0b11_01_01_00
    {0, 0, 0, 0}, // 0b11_01_01_01
    {0, 0, 0, 0}, // 0b11_01_01_10
    {0, 0, 0, 0}, // 0b11_01_01_11
    {2, 1, 3, 2}, // 0b11_10_00_00
    {2, 1, 3, 2}, // 0b11_10_00_01
    {0, 0, 0, 0}, // 0b11_10_00_10
    {2, 1, 3, 2}, // 0b11_10_00_11
    {2, 1, 3, 2}, // 0b11_10_01_00
    {0, 0, 0, 0}, // 0b11_10_01_01
    {0, 0, 0, 0}, // 0b11_10_01_10
    {2, 1, 3, 2}, // 0b11_10_01_11
    {0, 0, 0, 0}, // 0b11_11_00_00
    {0, 0, 0, 0}, // 0b11_11_00_01
    {3, 2, 2, 1}, // 0b11_11_00_10
    {0, 0, 0, 0}, // 0b11_11_00_11
    {0, 0, 0, 0}, // 0b11_11_01_00
    {0, 0, 0, 0}, // 0b11_11_01_01
    {3, 2, 2, 1}, // 0b11_11_01_10
    {0, 0, 0, 0}, // 0b11_11_01_11
    {1, 2, 2, 3}, // 0b10_00_00_00
    {1, 2, 2, 3}, // 0b10_00_00_01
    {0, 0, 0, 0}, // 0b10_00_00_10
    {1, 2, 2, 3}, // 0b10_00_00_11
    {1, 2, 2, 3}, // 0b10_00_01_00
    {0, 0, 0, 0}, // 0b10_00_01_01
    {0, 0, 0, 0}, // 0b10_00_01_10
    {1, 2, 2, 3}, // 0b10_00_01_11
    {1, 2, 2, 3}, // 0b10_01_00_00
    {0, 0, 0, 0}, // 0b10_01_00_01
    {0, 0, 0, 0}, // 0b10_01_00_10
    {1, 2, 2, 3}, // 0b10_01_00_11
    {0, 0, 0, 0}, // 0b10_01_01_00
    {0, 0, 0, 0}, // 0b10_01_01_01
    {0, 0, 0, 0}, // 0b10_01_01_10
    {0, 0, 0, 0}, // 0b10_01_01_11
    {0, 0, 0, 0}, // 0b10_10_00_00
    {0, 0, 0, 0}, // 0b10_10_00_01
    {0, 0, 0, 0}, // 0b10_10_00_10
    {0, 0, 0, 0}, // 0b10_10_00_11
    {0, 0, 0, 0}, // 0b10_10_01_00
    {0, 0, 0, 0}, // 0b10_10_01_01
    {0, 0, 0, 0}, // 0b10_10_01_10
    {0, 0, 0, 0}, // 0b10_10_01_11
    {1, 2, 2, 3}, // 0b10_11_00_00
    {1, 2, 2, 3}, // 0b10_11_00_01
    {0, 0, 0, 0}, // 0b10_11_00_10
    {1, 2, 2, 3}, // 0b10_11_00_11
    {1, 2, 2, 3}, // 0b10_11_01_00
    {0, 0, 0, 0}, // 0b10_11_01_01
    {0, 0, 0, 0}, // 0b10_11_01_10
    {1, 2, 2, 3}, // 0b10_11_01_11
    {0, 0, 0, 0}, // 0b11_00_00_00
    {0, 0, 0, 0}, // 0b11_00_00_01
    {3, 2, 2, 1}, // 0b11_00_00_10
    {0, 0, 0, 0}, // 0b11_00_00_11
    {0, 0, 0, 0}, // 0b11_00_01_00
    {0, 0, 0, 0}, // 0b11_00_01_01
    {3, 2, 2, 1}, // 0b11_00_01_10
    {0, 0, 0, 0}, // 0b11_00_01_11
    {0, 0, 0, 0}, // 0b11_01_00_00
    {0, 0, 0, 0}, // 0b11_01_00_01
    {3, 2, 2, 1}, // 0b11_01_00_10
    {0, 0, 0, 0}, // 0b11_01_00_11
    {0, 0, 0, 0}, // 0b11_01_01_00
    {0, 0, 0, 0}, // 0b11_01_01_01
    {0, 0, 0, 0}, // 0b11_01_01_10
    {0, 0, 0, 0}, // 0b11_01_01_11
    {2, 1, 3, 2}, // 0b11_10_00_00
    {2, 1, 3, 2}, // 0b11_10_00_01
    {0, 0, 0, 0}, // 0b11_10_00_10
    {2, 1, 3, 2}, // 0b11_10_00_11
    {2, 1, 3, 2}, // 0b11_10_01_00
    {0, 0, 0, 0}, // 0b11_10_01_01
    {0, 0, 0, 0}, // 0b11_10_01_10
    {2, 1, 3, 2}, // 0b11_10_01_11
    {0, 0, 0, 0}, // 0b11_11_00_00
    {0, 0, 0, 0}, // 0b11_11_00_01
    {3, 2, 2, 1}, // 0b11_11_00_10
    {0, 0, 0, 0}, // 0b11_11_00_11
    {0, 0, 0, 0}, // 0b11_11_01_00
    {0, 0, 0, 0}, // 0b11_11_01_01
    {3, 2, 2, 1}, // 0b11_11_01_10
    {0, 0, 0, 0}, // 0b11_11_01_11
};

/* bunch of macros for operating on tiles */
#define CSCOIN_SHORTEST_PATH_BUILD_TILE(a,b,c,d) (a << 6 | b << 4 | c << 2 | d)
#define CSCOIN_SHORTEST_PATH_TILE_TYPE_AT_POSITION(tile,pos) ((tile >> 2 * (3 - pos)) & 0xf)

static void
solve_shortest_path_challenge (CSCoinMT64 *mt64,
                               SHA256_CTX *checksum,
                               gint        grid_size,
                               gint        nb_blockers)
{
    // TODO
    guint8 cost;

    /* cost for reaching exit located at (1, 1) in a 4x4 tile entering in up-left position */
    cost = CSCOIN_SHORTEST_PATH_TILE_COST_PER_POSITION
        [CSCOIN_SHORTEST_PATH_BUILD_TILE (CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK, CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK,
                                          CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK, CSCOIN_SHORTEST_PATH_TILE_TYPE_EXIT)]
        [CSCOIN_SHORTEST_PATH_POSITION_UP_LEFT];

    /*
     * be
     * bf
     */
    guint8 *tiles =
    {
        CSCOIN_SHORTEST_PATH_BUILD_TILE (CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK, CSCOIN_SHORTEST_PATH_TILE_TYPE_EXIT,
                                         CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK, CSCOIN_SHORTEST_PATH_TILE_TYPE_FRONTIER)
    };

    CSCoinShortestPathTileType tile_type;

    /* type of the tile located at (0, 0) */
    tile_type = CSCOIN_SHORTEST_PATH_TILE_TYPE_AT_POSITION (tiles[0],
                                                            CSCOIN_SHORTEST_PATH_POSITION_UP_LEFT);

    g_assert (CSCOIN_SHORTEST_PATH_TILE_TYPE_BLANK == tile_type);
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
