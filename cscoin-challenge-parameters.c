#include "cscoin-challenge-parameters.h"

#include <string.h>

G_DEFINE_BOXED_TYPE (CSCoinChallengeParameters,
                     cscoin_challenge_parameters,
                     cscoin_challenge_parameters_copy,
                     cscoin_challenge_parameters_free);

CSCoinChallengeParameters *
cscoin_challenge_parameters_copy (CSCoinChallengeParameters *self)
{
    CSCoinChallengeParameters *ret = g_malloc (sizeof (CSCoinChallengeParameters));

    memcpy (ret, self, sizeof (CSCoinChallengeParameters));

    return ret;
}

void
cscoin_challenge_parameters_free (CSCoinChallengeParameters *self)
{
    g_free (self);
}
