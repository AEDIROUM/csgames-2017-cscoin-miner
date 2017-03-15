#ifndef __CSCOIN_SOLVER_H__
#define __CSCOIN_SOLVER_H__

#include <glib.h>
#include <gio/gio.h>

#include "cscoin-challenge-type.h"
#include "cscoin-challenge-parameters.h"

gchar * cscoin_solve_challenge (gint                        challenge_id,
                                CSCoinChallengeType         challenge_type,
                                const gchar                *last_solution_hash,
                                const gchar                *hash_prefix,
                                CSCoinChallengeParameters  *parameters,
                                GCancellable               *cancellable,
                                GError                    **error);

#endif /* __CSCOIN_SOLVER_H__ */
