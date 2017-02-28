#include <glib.h>

#include <stdlib.h>
#include <string.h>

#include "contrib/mt19937-64/mt64.h"

static gint
gint64cmp_asc (const void *a, const void *b)
{
    return *(guint64*) a < *(guint64*) b ? -1 : 1;
}

static gint
gint64cmp_desc (const void *a, const void *b)
{
    return *(guint64*) a < *(guint64*) b ? -1 : 1;
}

enum CSCoinChallengeType
{
    CSCOIN_CHALLENGE_SORTED_LIST,
    CSCOIN_CHALLENGE_REVERSE_SORTED_LIST
};

gchar *
cscoin_solve_challenge (gint         challenge_id,
                        const gchar *challenge_name,
                        const gchar *last_solution_hash,
                        const gchar *hash_prefix,
                        gint         nb_elements)
{
    g_autoptr (GChecksum) checksum = g_checksum_new (G_CHECKSUM_SHA256);
    union {
        guint8  digest[32];
        guint64 seed;
    } checksum_digest;
    gsize checksum_digest_len = 32;
    guint64 numbers[nb_elements];
    gint nonce = 0;
    gchar nonce_str[32];
    enum CSCoinChallengeType challenge_type;

    if (strcmp (challenge_name, "sorted_list") == 0)
    {
        challenge_type = CSCOIN_CHALLENGE_SORTED_LIST;
    }
    else if (strcmp (challenge_name, "reverse_sorted_list") == 0)
    {
        challenge_type = CSCOIN_CHALLENGE_REVERSE_SORTED_LIST;
    }
    else
    {
        g_return_val_if_reached (NULL);
    }

    while (TRUE)
    {
        g_snprintf (nonce_str, 32, "%d", nonce);

        g_checksum_update (checksum, (guchar*) last_solution_hash, 64);
        g_checksum_update (checksum, (guchar*) nonce_str, strlen (nonce_str));
        g_checksum_get_digest (checksum, checksum_digest.digest, &checksum_digest_len);
        g_checksum_reset (checksum);

        init_genrand64 (checksum_digest.seed);

        gint i;
        for (i = 0; i < nb_elements; i++)
        {
            numbers[i] = genrand64_int64 ();
        }

        switch (challenge_type)
        {
            case CSCOIN_CHALLENGE_SORTED_LIST:
                qsort (numbers, nb_elements, sizeof (guint64), gint64cmp_asc);
                break;
            case CSCOIN_CHALLENGE_REVERSE_SORTED_LIST:
                qsort (numbers, nb_elements, sizeof (guint64), gint64cmp_desc);
                break;
        }

        gchar number_str[32];
        for (i = 0; i < nb_elements; i++)
        {
            g_snprintf (number_str, 32, "%lu", numbers[i]);
            g_checksum_update (checksum, (guchar*) number_str, strlen (number_str));
        }

        const gchar * checksum_digest_str;
        checksum_digest_str = g_checksum_get_string (checksum);

        if (hash_prefix[0] == checksum_digest_str[0] &&
            hash_prefix[1] == checksum_digest_str[1] &&
            hash_prefix[2] == checksum_digest_str[2] &&
            hash_prefix[3] == checksum_digest_str[3])
        {
            return g_strdup (nonce_str);
        }
        else
        {
            g_checksum_reset (checksum);
            nonce++;
        }
    }
}
