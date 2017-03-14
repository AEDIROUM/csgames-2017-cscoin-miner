#ifndef __CSCOIN_HEAP64_H__
#define __CSCOIN_HEAP64_H__

#include <glib.h>

/* random sentinel (don't use this value explicitly) */
#define CSCOIN_HEAP64_NULL G_GUINT64_CONSTANT (12839182022384)

typedef struct _CSCoinHeap64 CSCoinHeap64;

struct _CSCoinHeap64
{
    guint64      *heap;
    gsize         heap_len;
    GCompareFunc  cmp;
};

CSCoinHeap64 * cscoin_heap64_new     (guint64      *heap,
                                      gsize         heap_len,
                                      GCompareFunc  cmp);
void           cscoin_heap64_free    (CSCoinHeap64 *self);
void           cscoin_heap64_init    (CSCoinHeap64 *self,
                                      guint64      *heap,
                                      gsize         heap_len,
                                      GCompareFunc  cmp);
gboolean       cscoin_heap64_push    (CSCoinHeap64 *self, guint64 val);
guint64        cscoin_heap64_pop     (CSCoinHeap64 *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (CSCoinHeap64, cscoin_heap64_free);

#endif /* __CSCOIN_HEAP64_H__ */
