#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/drvctlio.h>

#include "demi.h"
#include "device.h"

static int scan_system(struct demi_enumerate *de, const char *dev,
        demi_enumerate_cb cb, void *ptr)
{
    struct devlistargs laa = {0};
    struct demi_device dd;
    size_t i, children;

    strlcpy(laa.l_devname, dev, sizeof(laa.l_devname));

    do {
        if (ioctl(de->ctx->fd, DRVLISTDEV, &laa) == -1) {
            free(laa.l_childname);
            return -1;
        }

        if (laa.l_children == 0) {
            free(laa.l_childname);
            return 0;
        }

        children = laa.l_children;

        if (reallocarr(&laa.l_childname, children,
                sizeof(laa.l_childname[0])) != 0) {
            free(laa.l_childname);
            return -1;
        }

        if (ioctl(de->ctx->fd, DRVLISTDEV, &laa) == -1) {
            free(laa.l_childname);
            return -1;
        }
    }
    while (children != laa.l_children);

    for (i = 0; i < laa.l_children; i++) {
        // XXX continue?
        if (device_init(&dd, de->ctx, laa.l_childname[i], 0, 0, 0) == -1) {
            free(laa.l_childname);
            return -1;
        }

        if (cb(&dd, ptr) == -1) {
            free(laa.l_childname);
            return -1;
        }

        if (scan_system(de, laa.l_childname[i], cb, ptr) == -1) {
            free(laa.l_childname);
            return -1;
        }
    }

    free(laa.l_childname);
    return 0;
}

int demi_enumerate_scan_system(struct demi_enumerate *de,
        demi_enumerate_cb cb, void *ptr)
{
    return de && cb ? scan_system(de, "", cb, ptr) : -1;
}

int demi_enumerate_init(struct demi_enumerate *de, struct demi *ctx)
{
    if (!de || !ctx) {
        return -1;
    }

    de->ctx = ctx;
    return 0;
}

void demi_enumerate_finish(struct demi_enumerate *de)
{
    (void)de;
}
