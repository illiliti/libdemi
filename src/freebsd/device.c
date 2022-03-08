#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>

#include "demi.h"
#include "evdev.h"
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

static inline enum demi_class parse_class(const char *devname)
{
    enum demi_class class;

    // TODO lookup table
    if (strncmp(devname, "input/", 6) == 0) {
        class = DEMI_CLASS_INPUT;
    }
    else if (strncmp(devname, "dri/", 4) == 0) {
        class = DEMI_CLASS_DRM;
    }
    else {
        class = DEMI_CLASS_UNKNOWN;
    }

    return class;
}

int demi_device_get_class(struct demi_device *dd, enum demi_class *class)
{
    const char *devname;

    if (!dd || !class) {
        errno = EINVAL;
        return -1;
    }

    if (dd->class) {
        *class = dd->class;
        return 0;
    }

    if (demi_device_get_devname(dd, &devname) == -1) {
        return -1;
    }

    dd->class = parse_class(devname);
    *class = dd->class;
    return 0;
}

// TODO simplify
static inline int fetch_evdev(struct evdev *evdev, uint32_t unit)
{
    char mib[48];
    size_t len;

    // https://cgit.freebsd.org/src/commit/sys/dev/evdev/evdev.c?id=f99e7b1aed7ea65ca0dbe5b182f2b9cbfdfe54db

    len = sizeof(evdev->ev);
    snprintf(mib, sizeof(mib), "kern.evdev.input.%" PRIu32 ".type_bits", unit);
    if (sysctlbyname(mib, &evdev->ev, &len, NULL, 0) == -1) {
        return -1;
    }

    len = sizeof(evdev->key);
    snprintf(mib, sizeof(mib), "kern.evdev.input.%" PRIu32 ".key_bits", unit);
    if (sysctlbyname(mib, &evdev->key, &len, NULL, 0) == -1) {
        return -1;
    }

    len = sizeof(evdev->rel);
    snprintf(mib, sizeof(mib), "kern.evdev.input.%" PRIu32 ".rel_bits", unit);
    if (sysctlbyname(mib, &evdev->rel, &len, NULL, 0) == -1) {
        return -1;
    }

    len = sizeof(evdev->abs);
    snprintf(mib, sizeof(mib), "kern.evdev.input.%" PRIu32 ".abs_bits", unit);
    if (sysctlbyname(mib, &evdev->abs, &len, NULL, 0) == -1) {
        return -1;
    }

    len = sizeof(evdev->prop);
    snprintf(mib, sizeof(mib), "kern.evdev.input.%" PRIu32 ".props", unit);
    if (sysctlbyname(mib, &evdev->prop, &len, NULL, 0) == -1) {
        return -1;
    }

    return 0;
}

// TODO figure out how to check boot vga(if possible at all)
int demi_device_get_type(struct demi_device *dd, uint32_t *type)
{
    enum demi_class class;
    struct evdev evdev;
    uint32_t unit;

    if (!dd || !type) {
        errno = EINVAL;
        return -1;
    }

    if (dd->type) {
        *type = dd->type;
        return 0;
    }

    if (demi_device_get_class(dd, &class) == -1) {
        return -1;
    }

    if (class != DEMI_CLASS_INPUT) {
        errno = ENOENT;
        return -1;
    }

    if (demi_device_get_devunit(dd, &unit) == -1) {
        return -1;
    }

    if (fetch_evdev(&evdev, unit) == -1) {
        return -1;
    }

    dd->type = parse_evdev(&evdev);
    *type = dd->type;
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

static int device_init(struct demi_device *dd, struct demi *ctx,
        const char *devname, dev_t devnum, mode_t type)
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
