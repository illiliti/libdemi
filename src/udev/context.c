#include <libudev.h>

#include "demi.h"

int demi_init(struct demi *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->udev = udev_new();
    return ctx->udev ? 0 : -1;
}

void demi_finish(struct demi *ctx)
{
    if (!ctx) {
        return;
    }

    udev_unref(ctx->udev);
}
