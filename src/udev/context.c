#include <stdlib.h>
#include <libudev.h>

#include "demi.h"
#include "udev.h"

struct demi *demi_new(void)
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

    ctx->ref = 1;
    return ctx;
}

struct demi *demi_ref(struct demi *ctx)
{
    if (!ctx) {
        return NULL;
    }

    ctx->ref++;
    return ctx;
}

void demi_unref(struct demi *ctx)
{
    if (!ctx) {
        return;
    }

    if (--ctx->ref > 0) {
        return;
    }
    
    udev_unref(ctx->udev);
    free(ctx);
}
