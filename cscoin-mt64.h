#ifndef __CSCOIN_MT64_H__
#define __CSCOIN_MT64_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _CSCoinMT64 CSCoinMT64;

struct _CSCoinMT64
{
    gint    index;
    guint64 mt[312];
};

CSCoinMT64 * cscoin_mt64_new         (void);
void         cscoin_mt64_free        (CSCoinMT64 *self);
void         cscoin_mt64_init        (CSCoinMT64 *self);
void         cscoin_mt64_set_seed    (CSCoinMT64 *self, guint64 seed);
guint64      cscoin_mt64_next_uint64 (CSCoinMT64 *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CSCoinMT64, cscoin_mt64_free);

G_END_DECLS

#endif /* __CSCOIN_MT64_H__ */
