#include <stdlib.h>
#include <libudev.h>

#include "demi.h"
#include "udev.h"

struct demi *demi_init(void)
{
    struct demi *ctx;

    ctx = malloc(sizeof(*ctx));

    if (!ctx) {
        return NULL;
    }

    ctx->udev = udev_new();

    if (!ctx->udev) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void demi_deinit(struct demi *ctx)
{
    if (!ctx) {
        return;
    }
    
    udev_unref(ctx->udev);
    free(ctx);
}
