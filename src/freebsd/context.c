#include "demi.h"

int demi_init(struct demi *ctx)
{
    return ctx ? 0 : -1;
}

void demi_finish(struct demi *ctx)
{
    (void)ctx;
}
