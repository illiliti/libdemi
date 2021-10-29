#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "demi.h"
#include "linux.h"

static int scan_system(struct demi_enumerate *de, const char *path,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    struct demi_device *dd;
    struct dirent *dt;
    char syspath[48];
    DIR *dp;

    dp = opendir(path);

    if (!dp) {
        return -1;
    }

    while ((dt = readdir(dp))) {
        if (dt->d_name[0] == '.') {
            continue;
        }

        // TODO use memcpy
        snprintf(syspath, sizeof(syspath), "%s/%s", path, dt->d_name);
        dd = device_init_syspath(de->ctx, syspath);

        // XXX continue?
        if (!dd) {
            closedir(dp);
            return -1;
        }

        if (cb(dd, ptr) == -1) {
            closedir(dp);
            return -1;
        }
    }

    closedir(dp);
    return 0;
}

int demi_enumerate_scan_system(struct demi_enumerate *de,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    static const char *path[] = { "/sys/dev/block", "/sys/dev/char", NULL };
    int i;

    if (!de || !cb) {
        return -1;
    }

    for (i = 0; path[i]; i++) {
        if (scan_system(de, path[i], cb, ptr) == -1) {
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
