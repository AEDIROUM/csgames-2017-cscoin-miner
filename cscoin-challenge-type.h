#ifndef __CSCOIN_CHALLENGE_TYPE_H__
#define __CSCOIN_CHALLENGE_TYPE_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum _CSCoinChallengeType CSCoinChallengeType;

enum _CSCoinChallengeType
{
    CSCOIN_CHALLENGE_TYPE_SORTED_LIST,
    CSCOIN_CHALLENGE_TYPE_REVERSE_SORTED_LIST,
    CSCOIN_CHALLENGE_TYPE_SHORTEST_PATH
};

#define CSCOIN_TYPE_CHALLENGE_TYPE cscoin_challenge_type_get_type ()
GType cscoin_challenge_type_get_type ();

G_END_DECLS

#endif /* __CSCOIN_CHALLENGE_TYPE_H__ */
