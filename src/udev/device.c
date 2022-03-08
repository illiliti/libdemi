#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <libudev.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "demi.h"
#include "device.h"

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    const char *node;

    if (!dd || !devnode) {
        errno = EINVAL;
        return -1;
    }

    node = udev_device_get_devnode(dd->udev_device);

    if (!node) {
        errno = ENOENT;
        return -1;
    }

    *devnode = node;
    return 0;
}

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    const char *name;

    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    name = udev_device_get_sysname(dd->udev_device);

    if (!name) {
        errno = ENOENT;
        return -1;
    }

    *devname = name;
    return 0;
}

int demi_device_get_devnum(struct demi_device *dd, dev_t *devnum)
{
    dev_t num;

    if (!dd || !devnum) {
        errno = EINVAL;
        return -1;
    }

    num = udev_device_get_devnum(dd->udev_device);

    // XXX
    if (num == makedev(0, 0)) {
        errno = ENOENT;
        return -1;
    }

    *devnum = num;
    return 0;
}

int demi_device_get_devunit(struct demi_device *dd, uint32_t *devunit)
{
    const char *sysnum;

    if (!dd || !devunit) {
        errno = EINVAL;
        return -1;
    }

    sysnum = udev_device_get_sysnum(dd->udev_device);

    if (!sysnum) {
        errno = ENOENT;
        return -1;
    }

    *devunit = (uint32_t)strtoul(sysnum, NULL, 10);
    return 0;
}

int demi_device_get_seat(struct demi_device *dd, const char **seat)
{
    const char *value;

    if (!dd || !seat) {
        errno = EINVAL;
        return -1;
    }

    value = udev_device_get_property_value(dd->udev_device, "ID_SEAT");

    if (!value) {
        errno = ENOENT;
        return -1;
    }

    *seat = value;
    return 0;
}

int demi_device_get_action(struct demi_device *dd, enum demi_action *action)
{
    const char *event;

    if (!dd || !action) {
        errno = EINVAL;
        return -1;
    }

    if (dd->action) {
        *action = dd->action;
        return 0;
    }

    event = udev_device_get_action(dd->udev_device);

    if (!event) {
        errno = ENOENT;
        return -1;
    }

    // TODO lookup table
    if (strcmp(event, "add") == 0) {
        dd->action = DEMI_ACTION_ATTACH;
    }
    else if (strcmp(event, "remove") == 0) {
        dd->action = DEMI_ACTION_DETACH;
    }
    else if (strcmp(event, "change") == 0) {
        dd->action = DEMI_ACTION_CHANGE;
    }
    else {
        dd->action = DEMI_ACTION_UNKNOWN;
    }

    *action = dd->action;
    return 0;
}

int device_init(struct demi_device *dd, struct demi *ctx,
        struct udev_device *udev_device)
{
    memset(dd, 0, sizeof(*dd));

    dd->udev_device = udev_device;
    dd->ctx = ctx;
    return 0;
}

int demi_device_init_devnum(struct demi_device *dd, struct demi *ctx,
        dev_t devnum, mode_t type)
{
    struct udev_device *udev_device;

    if (!dd || !ctx) {
        return -1;
    }

    switch (type) {
    case S_IFCHR:
        udev_device = udev_device_new_from_devnum(ctx->udev, 'c', devnum);
        break;
    case S_IFBLK:
        udev_device = udev_device_new_from_devnum(ctx->udev, 'b', devnum);
        break;
    default:
        return -1;
    }

    if (!udev_device) {
        return -1;
    }

    if (device_init(dd, ctx, udev_device) == -1) {
        udev_device_unref(udev_device);
        return -1;
    }

    return 0;
}

int demi_device_init_devnode(struct demi_device *dd, struct demi *ctx,
        const char *devnode)
{
    struct stat st;

    if (!dd || !ctx || !devnode) {
        return -1;
    }

    if (stat(devnode, &st) == -1) {
        return -1;
    }

    st.st_mode = st.st_mode & (S_IFBLK | S_IFCHR);
    return demi_device_init_devnum(dd, ctx, st.st_rdev, st.st_mode);
}

void demi_device_finish(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    udev_device_unref(dd->udev_device);
}
