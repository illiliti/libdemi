#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/drvctlio.h>

#include "demi.h"
#include "netbsd.h"

static int scan_system(struct demi_enumerate *de, const char *dev,
       int (*cb)(struct demi_device *, void *), void *ptr)
{
    struct devlistargs laa = {0};
    struct demi_device *dd;
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
        dd = device_new(de->ctx, NULL, laa.l_childname[i], 0, 0, 0);

        // XXX continue?
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

    return 0;
}

int demi_enumerate_scan_system(struct demi_enumerate *de,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    return de && cb ? scan_system(de, "", cb, ptr) : -1;
}

struct demi_enumerate *demi_enumerate_new(struct demi *ctx)
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
    de->ref = 1;
    return de;
}

struct demi_enumerate *demi_enumerate_ref(struct demi_enumerate *de)
{
    if (!de) {
        return NULL;
    }

    de->ref++;
    return de;
}

void demi_enumerate_unref(struct demi_enumerate *de)
{
    if (!de) {
        return;
    }

    if (--de->ref > 0) {
        return;
    }

    free(de);
}
