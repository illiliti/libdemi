#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "demi.h"
#include "device.h"

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    size_t len;

    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devname[0] != '\0') {
        *devname = dd->devname;
        return 0;
    }

    len = sizeof(dd->devname);
    switch (devname_r(dd->devnum, dd->devtype, dd->devname, len)) {
    case 0:
        *devname = dd->devname;
        return 0;
    case ENOENT:
        errno = ENOENT;
        break;
    default:
        break;
    }

    return -1;
}

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    const char *devname;

    if (!dd || !devnode) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devnode[0] != '\0') {
        *devnode = dd->devnode;
        return 0;
    }

    if (demi_device_get_devname(dd, &devname) == -1) {
        return -1;
    }

    strlcpy(dd->devnode, "/dev/", sizeof(dd->devnode));
    strlcat(dd->devnode, devname, sizeof(dd->devnode));
    *devnode = dd->devnode;
    return 0;
}

int demi_device_get_devnum(struct demi_device *dd, dev_t *devnum)
{
    const char *devnode;
    struct stat st;

    if (!dd || !devnum) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devnum) {
        *devnum = dd->devnum;
        return 0;
    }

    if (demi_device_get_devnode(dd, &devnode) == -1) {
        return -1;
    }

    if (stat(devnode, &st) == -1) {
        errno = ENOENT;
        return -1;
    }

    dd->devtype = st.st_mode & (S_IFCHR | S_IFBLK);
    dd->devnum = st.st_rdev;

    *devnum = dd->devnum;
    return 0;
}

// TODO use DRVCTLCOMMAND to get unit
int demi_device_get_devunit(struct demi_device *dd, uint32_t *devunit)
{
    const char *devname;
    size_t i;

    if (!dd || !devunit) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devunit != -1) {
        *devunit = (uint32_t)dd->devunit;
        return 0;
    }

    if (demi_device_get_devname(dd, &devname)) {
        return -1;
    }

    for (i = 0; devname[i] != '\0'; i++) {
        if (devname[i] >= '0' && devname[i] <= '9') {
            dd->devunit = (int32_t)strtol(devname + i, NULL, 10);
            *devunit = (uint32_t)dd->devunit;
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

int demi_device_get_seat(struct demi_device *dd, const char **seat)
{
    if (!dd || !seat) {
        errno = EINVAL;
        return -1;
    }

    errno = ENOTSUP;
    return -1;
}

int demi_device_get_action(struct demi_device *dd, enum demi_action *action)
{
    if (!dd || !action) {
        errno = EINVAL;
        return -1;
    }

    if (!dd->action) {
        errno = ENOENT;
        return -1;
    }

    *action = dd->action;
    return 0;
}

int device_init(struct demi_device *dd, struct demi *ctx, const char *devname,
        dev_t devnum, mode_t type, enum demi_action action)
{
    size_t len;

    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
    dd->action = action;
    dd->devnum = devnum;
    dd->devtype = type;
    dd->devunit = -1;

    if (!devname) {
        return 0;
    }

    len = sizeof(dd->devname);

    if (strlcpy(dd->devname, devname, len) >= len) {
        return -1;
    }

    return 0;
}

int demi_device_init_devnode(struct demi_device *dd, struct demi *ctx,
        const char *devnode)
{
    if (!dd || !ctx || !devnode) {
        return -1;
    }

    if (strncmp(devnode, "/dev/", 5) != 0) {
        return -1;
    }

    return device_init(dd, ctx, devnode + 5, 0, 0, 0);
}

int demi_device_init_devnum(struct demi_device *dd, struct demi *ctx,
        dev_t devnum, mode_t type)
{
    if (!dd || !ctx) {
        return -1;
    }

    if (type != S_IFCHR && type != S_IFBLK) {
        return -1;
    }

    return device_init(dd, ctx, NULL, devnum, type, 0);
}

void demi_device_finish(struct demi_device *dd)
{
    (void)dd;
}
