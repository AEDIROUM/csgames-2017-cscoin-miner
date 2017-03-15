#ifndef __CSCOIN_CHALLENGE_PARAMETERS_H__
#define __CSCOIN_CHALLENGE_PARAMETERS_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

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

#define CSCOIN_TYPE_CHALLENGE_PARAMETERS cscoin_challenge_parameters_get_type ()
GType    cscoin_challenge_parameters_get_type ();
gpointer cscoin_challenge_parameters_copy     (gpointer boxed);
void     cscoin_challenge_parameters_free     (gpointer boxed);

G_END_DECLS

#endif /* __CSCOIN_CHALLENGE_PARAMETERS_H__ */
