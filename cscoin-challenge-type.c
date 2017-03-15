#include "cscoin-challenge-type.h"

static const GEnumValue
CHALLENGE_TYPE_ENUM_VALUES[] =
{
    {0, "CSCOIN_CHALLENGE_TYPE_SORTED_LIST",         "sorted_list"},
    {1, "CSCOIN_CHALLENGE_TYPE_REVERSE_SORTED_LIST", "reverse_sorted_list"},
    {2, "CSCOIN_CHALLENGE_TYPE_SHORTEST_PATH",       "shortest_path"},
    NULL
};

static GType
cscoin_challenge_type = 0;

GType
cscoin_challenge_type_get_type ()
{
    if (g_once_init_enter (&cscoin_challenge_type))
    {
        GType cscoin_challenge_type_value = g_enum_register_static ("CSCoinChallengeType", CHALLENGE_TYPE_ENUM_VALUES);
        g_once_init_leave (&cscoin_challenge_type, cscoin_challenge_type_value);
    }

    return cscoin_challenge_type;
}
