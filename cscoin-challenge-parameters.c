#include "cscoin-challenge-parameters.h"

#include <string.h>

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
