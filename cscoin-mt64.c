#include "cscoin-mt64.h"

#include <stdint.h>
#include <string.h>

#define CSCOIN_MT64_W 64
#define CSCOIN_MT64_N 312
#define CSCOIN_MT64_M 156
#define CSCOIN_MT64_R 31
#define CSCOIN_MT64_A UINT64_C (0xB5026F5AA96619E9)
#define CSCOIN_MT64_U 29
#define CSCOIN_MT64_D UINT64_C (0x5555555555555555)
#define CSCOIN_MT64_S 17
#define CSCOIN_MT64_B UINT64_C (0x71D67FFFEDA60000)
#define CSCOIN_MT64_T 37
#define CSCOIN_MT64_C UINT64_C (0xFFF7EEE000000000)
#define CSCOIN_MT64_L 43
#define CSCOIN_MT64_F UINT64_C (6364136223846793005)

#define CSCOIN_MT64_UPPER_MASK UINT64_C (0xFFFFFFFF80000000)
#define CSCOIN_MT64_LOWER_MASK UINT64_C (0x7FFFFFFF)

CSCoinMT64 *
cscoin_mt64_new (void)
{
    return g_malloc (sizeof (CSCoinMT64));
}

void
cscoin_mt64_free (CSCoinMT64 *self)
{
    g_free (self);
}

void
cscoin_mt64_init (CSCoinMT64 *self)
{
    self->index = 0;
    memset(self->mt, 0, sizeof (self->mt));
}

void
cscoin_mt64_set_seed (CSCoinMT64 *self, guint64 seed)
{
    gint i;

    self->index = CSCOIN_MT64_N;
    self->mt[0] = seed;

    for (i = 1; i < CSCOIN_MT64_N; i++)
    {
        self->mt[i] = CSCOIN_MT64_F * (self->mt[i - 1] ^ (self->mt[i - 1] >> (CSCOIN_MT64_W - 2))) + i;
    }
}

guint64
cscoin_mt64_next_uint64 (CSCoinMT64 *self)
{
    gint i;
    guint64 x;
    guint64 x_a;
    guint64 y;

    if (G_UNLIKELY (self->index >= CSCOIN_MT64_N))
    {
        for (i = 0; i < CSCOIN_MT64_N; i++)
        {
            x = (self->mt[i] & CSCOIN_MT64_UPPER_MASK) + (self->mt[(i + 1) % CSCOIN_MT64_N] & CSCOIN_MT64_LOWER_MASK);

            x_a = x >> 1;

            if (x % 2)
            {
                x_a ^= CSCOIN_MT64_A;
            }

            self->mt[i] = self->mt[(i + CSCOIN_MT64_M) % CSCOIN_MT64_N] ^ x_a;
        }

        self->index = 0;
    }

    y = self->mt[self->index++];

    y ^= (y >> CSCOIN_MT64_U) & CSCOIN_MT64_D;
    y ^= (y << CSCOIN_MT64_S) & CSCOIN_MT64_B;
    y ^= (y << CSCOIN_MT64_T) & CSCOIN_MT64_C;
    y ^= (y >> CSCOIN_MT64_L);

    return y;
}
