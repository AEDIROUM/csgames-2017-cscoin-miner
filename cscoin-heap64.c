#include "cscoin-heap64.h"

CSCoinHeap64 *
cscoin_heap64_new (guint64 *heap, gsize heap_len, GCompareFunc cmp)
{
    CSCoinHeap64 *self;

    self = g_malloc (sizeof (CSCoinHeap64));

    cscoin_heap64_init (self, heap, heap_len, cmp);

    return self;
}

void
cscoin_heap64_init (CSCoinHeap64 *self, guint64 *heap, gsize heap_len, GCompareFunc cmp)
{
    g_return_if_fail (self != NULL);

    self->heap     = heap;
    self->heap_len = heap_len;
    self->cmp      = cmp;

    gint i;
    for (i = 0; i < self->heap_len; i++)
    {
        self->heap[i] = CSCOIN_HEAP64_NULL;
    }
}

gboolean
cscoin_heap64_push (CSCoinHeap64 *self, guint64 val)
{
    gint i;
    guint64 tmp;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (val != CSCOIN_HEAP64_NULL, FALSE);

    i = 0;
    while (self->heap[i++] != CSCOIN_HEAP64_NULL);

    self->heap[i - 1] = val;

    /* heapify */
    while (i > 1)
    {
        /* swap parent & child */
        if (self->cmp (self->heap + ((i - 1) / 2), self->heap + i) < 0)
        {
            tmp = self->heap[(i - 1) / 2];
            self->heap[(i - 1) / 2] = tmp;
            self->heap[i] = tmp;
        }
        i--;
    }

    return TRUE;
}

guint64
cscoin_heap64_pop (CSCoinHeap64 *self)
{
    gint i;
    guint64 ret;

    ret = self->heap[0];

    for (i = 0; i < self->heap_len && self->heap[i] != CSCOIN_HEAP64_NULL;)
    {
        if (self->cmp (self->heap + 2 * i + 1, self->heap + 2 * i + 2) < 0)
        {
            // raise left
            self->heap[i] = self->heap[2 * i + 1];
            i = (2 * i) + 1;
        }
        else
        {
            // raise right
            self->heap[i] = self->heap[2 * i + 2];
            i = (2 * i) + 2;
        }
    }

    return ret;
}
