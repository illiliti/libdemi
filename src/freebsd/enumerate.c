#include <fts.h>
#include <assert.h>
#include <stddef.h>
#include <sys/stat.h>

#include "demi.h"
#include "device.h"

int demi_enumerate_scan_system(struct demi_enumerate *de,
        demi_enumerate_cb cb, void *ptr)
{
    static char *const path[] = { "/dev", NULL };
    struct demi_device dd;
    const char *devname;
    size_t dev_len;
    mode_t type;
    FTSENT *fe;
    FTS *fp;

    fp = fts_open(path, FTS_NOCHDIR | FTS_PHYSICAL, NULL);

    if (!fp) {
        return -1;
    }

    while ((fe = fts_read(fp))) {
        if (fe->fts_info != FTS_DEFAULT) {
            continue;
        }

        type = fe->fts_statp->st_mode & (S_IFCHR | S_IFBLK);

        if (!type) {
            continue;
        }

        dev_len = sizeof("/dev/") - 1;
        assert(fe->fts_pathlen > dev_len);
        devname = fe->fts_path + dev_len;

        // XXX continue?
        if (device_init(&dd, de->ctx, devname,
                fe->fts_statp->st_rdev, type) == -1) {
            fts_close(fp);
            return -1;
        }

        if (cb(&dd, ptr) == -1) {
            fts_close(fp);
            return -1;
        }
    }

    fts_close(fp);
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
