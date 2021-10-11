#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "demi.h"
#include "linux.h"

static int filter_dot(const struct dirent *dt)
{
    return dt->d_name[0] != '.';
}

static int scan_devices(struct demi_enumerate *de, const char *path,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    struct demi_device *dd;
    struct dirent **dt;
    int i, n, ret = 0;
    char syspath[48];

    // XXX readdir vs readdir_r vs scandir
    n = scandir(path, &dt, filter_dot, NULL);

    if (n == -1) {
        return -1;
    }

    for (i = 0; i < n; i++) {
        snprintf(syspath, sizeof(syspath), "%s/%s", path, dt[i]->d_name);
        dd = device_init_syspath(de->ctx, syspath);

        if (!dd) {
            ret = -1;
            break;
        }

        if (cb(dd, ptr) == -1) {
            ret = -1;
            break;
        }
    }

    for (i = 0; i < n; i++) {
        free(dt[i]);
    }

    free(dt);
    return ret;
}

int demi_enumerate_scan_devices(struct demi_enumerate *de,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    static const char *path[] = { "/sys/dev/block", "/sys/dev/char", NULL };
    int i;

    if (!de || !cb) {
        return -1;
    }

    for (i = 0; path[i]; i++) {
        if (scan_devices(de, path[i], cb, ptr) == -1) {
            return -1;
        }
    }

    return 0;
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
