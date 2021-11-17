#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/drvctlio.h>

#include "demi.h"
#include "netbsd.h"

struct demi *demi_new(void)
{
    struct demi *ctx;

    ctx = malloc(sizeof(*ctx));

    if (!ctx) {
        return NULL;
    }

    ctx->fd = open(DRVCTLDEV, O_RDWR | O_CLOEXEC | O_NONBLOCK);

    if (ctx->fd == -1) {
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

    close(ctx->fd);
    free(ctx);
}
