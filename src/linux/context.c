#include <stdlib.h>

#include "demi.h"
#include "linux.h"

struct demi *demi_init(void)
{
    struct demi *ctx;

    ctx = malloc(sizeof(*ctx));

    if (!ctx) {
        return NULL;
    }

    return ctx;
}

void demi_deinit(struct demi *ctx)
{
    free(ctx);
}
