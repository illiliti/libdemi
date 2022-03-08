#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "demi.h"
#include "device.h"

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devname[0] != '\0') {
        *devname = dd->devname;
        return 0;
    }

    // XXX devname_r doesn't have error handling
    devname_r(dd->devnum, dd->devtype, dd->devname, sizeof(dd->devname));
    *devname = dd->devname;
    return 0;
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

static inline void parse_var(struct demi_device *dd, const char *line)
{
    const char *value;

    value = strchr(line, '=');

    if (!value) {
        return;
    }

    value += 1; // strip leading '='

    if (dd->devname[0] == '\0' && strncmp(line, "cdev", 4) == 0) {
        strlcpy(dd->devname, value, sizeof(dd->devname));
    }
    else if (strncmp(line, "type", 4) != 0) {
        return;
    }
    else if (strcmp(value, "CREATE") == 0) {
        dd->action = DEMI_ACTION_ATTACH;
    }
    else if (strcmp(value, "DESTROY") == 0) {
        dd->action = DEMI_ACTION_DETACH;
    }
    else if (strcmp(value, "HOTPLUG") == 0) {
        dd->action = DEMI_ACTION_CHANGE;
    }
    else {
        dd->action = DEMI_ACTION_UNKNOWN;
    }
}

int device_init_msg(struct demi_device *dd, struct demi *ctx,
        const char *buf, size_t len)
{
    const char *end;

    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
    dd->devunit = -1;

    for (end = buf + len; buf < end; buf += strlen(buf) + 1) {
        parse_var(dd, buf);
    }

    return 0;
}

int device_init(struct demi_device *dd, struct demi *ctx, const char *devname,
        dev_t devnum, mode_t type)
{
    size_t len;

    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
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

    return device_init(dd, ctx, devnode + 5, 0, 0);
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

    return device_init(dd, ctx, NULL, devnum, type);
}

void demi_device_finish(struct demi_device *dd)
{
    (void)dd;
}
