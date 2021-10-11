#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/drvctlio.h>

#include "demi.h"
#include "netbsd.h"

struct demi *demi_init(void)
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

    return ctx;
}

void demi_deinit(struct demi *ctx)
{
    if (!ctx) {
        return;
    }

    close(ctx->fd);
    free(ctx);
}
