#ifndef __CSCOIN_SOLVER_H__
#define __CSCOIN_SOLVER_H__

#include <glib.h>
#include <gio/gio.h>

gchar * cscoin_solve_challenge (gint           challenge_id,
                                const gchar   *challenge_name,
                                const gchar   *last_solution_hash,
                                const gchar   *hash_prefix,
                                gint           nb_elements,
                                GCancellable  *cancellable,
                                GError       **error);

#endif /* __CSCOIN_SOLVER_H__ */
