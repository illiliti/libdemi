#include <fcntl.h>
#include <unistd.h>
#include <sys/drvctlio.h>

#include "demi.h"

int demi_init(struct demi *ctx)
{
    if (!ctx) {
        return -1;
    }

    ctx->fd = open(DRVCTLDEV, O_RDWR | O_CLOEXEC | O_NONBLOCK);
    return ctx->fd == -1 ? -1 : 0;
}

void demi_finish(struct demi *ctx)
{
    if (!ctx) {
        return;
    }

    close(ctx->fd);
}
