#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/drvctlio.h>

#include "demi.h"
#include "netbsd.h"

static int scan_system(struct demi_enumerate *de, const char *dev,
       int (*cb)(struct demi_device *, void *), void *ptr)
{
    struct devlistargs laa = {0};
    struct demi_device *dd;
    size_t i;

    strlcpy(laa.l_devname, dev, sizeof(laa.l_devname));

    if (ioctl(de->ctx->fd, DRVLISTDEV, &laa) == -1) {
        return -1;
    }

    if (laa.l_children == 0) {
        return 0;
    }

    laa.l_childname = calloc(laa.l_children, sizeof(laa.l_childname[0]));

    if (!laa.l_childname) {
        return -1;
    }

    if (ioctl(de->ctx->fd, DRVLISTDEV, &laa) == -1) {
        free(laa.l_childname);
        return -1;
    }

    for (i = 0; i < laa.l_children; i++) {
        dd = device_init(de->ctx, NULL, laa.l_childname[i], 0, 0,
                DEMI_ACTION_UNKNOWN);

        if (!dd) {
            free(laa.l_childname);
            return -1;
        }

        if (cb(dd, ptr) == -1) {
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
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    return de && cb ? scan_system(de, "", cb, ptr) : -1;
}

struct demi_enumerate *demi_enumerate_init(struct demi *ctx)
{
    struct demi_enumerate *de;

    if (!ctx) {
        return NULL;
    }

    de = malloc(sizeof(*de));

    if (!de) {
        return NULL;
    }

    de->ctx = ctx;
    return de;
}

void demi_enumerate_deinit(struct demi_enumerate *de)
{
    free(de);
}
