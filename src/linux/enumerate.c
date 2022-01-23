#include <dirent.h>

#include "demi.h"
#include "device.h"

static int scan_system(struct demi_enumerate *de, const char *path,
        demi_enumerate_cb cb, void *ptr)
{
    struct demi_device dd;
    struct dirent *dt;
    DIR *dp;
    int dfd;

    dp = opendir(path);

    if (!dp) {
        return -1;
    }

    dfd = dirfd(dp);

    while ((dt = readdir(dp))) {
        if (dt->d_name[0] == '.') {
            continue;
        }

        // XXX continue?
        if (device_init_syspath(&dd, de->ctx, dfd, dt->d_name) == -1) {
            closedir(dp);
            return -1;
        }

        if (cb(&dd, ptr) == -1) {
            closedir(dp);
            return -1;
        }
    }

    closedir(dp);
    return 0;
}

int demi_enumerate_scan_system(struct demi_enumerate *de, demi_enumerate_cb cb,
        void *ptr)
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
