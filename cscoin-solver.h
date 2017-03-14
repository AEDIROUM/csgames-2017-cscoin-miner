#ifndef __CSCOIN_SOLVER_H__
#define __CSCOIN_SOLVER_H__

#include <glib.h>
#include <gio/gio.h>

typedef union _CSCoinChallengeParameters CSCoinChallengeParameters;

union _CSCoinChallengeParameters
{
    struct
    {
        gint nb_elements;
    } sorted_list;
    struct
    {
        gint nb_elements;
    } reverse_sorted_list;
    struct
    {
        gint grid_size;
        gint nb_blockers;
    } shortest_path;
};

gchar * cscoin_solve_challenge (gint                        challenge_id,
                                const gchar                *challenge_name,
                                const gchar                *last_solution_hash,
                                const gchar                *hash_prefix,
                                CSCoinChallengeParameters  *parameters,
                                GCancellable               *cancellable,
                                GError                    **error);

#endif /* __CSCOIN_SOLVER_H__ */
