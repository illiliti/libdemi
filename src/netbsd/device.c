#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "demi.h"
#include "wscons.h"
#include "device.h"

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    size_t len = 1;

    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devname) {
        *devname = dd->devname;
        return 0;
    }

    while (1) {
        len += 48;

        if (reallocarr(&dd->devname, 1, len) != 0) {
            free(dd->devname);
            dd->devname = NULL;
            return -1;
        }

        switch (devname_r(dd->devnum, dd->devtype, dd->devname, len)) {
        case 0:
            *devname = dd->devname;
            return 0;
        case ERANGE:
            continue;
        case ENOENT:
            free(dd->devname);
            dd->devname = NULL;
            errno = ENOENT;
            return -1;
        default:
            abort();
        }
    }

    // XXX unreachable
    abort();
}

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    const char *devname;
    size_t len;

    if (!dd || !devnode) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devnode) {
        *devnode = dd->devnode;
        return 0;
    }

    if (demi_device_get_devname(dd, &devname) == -1) {
        return -1;
    }

    len = 5 + strlen(devname) + 1;
    dd->devnode = malloc(len);

    if (!dd->devnode) {
        return -1;
    }

    // TODO use memcpy
    snprintf(dd->devnode, len, "/dev/%s", devname);

    // FIXME TOCTOU
    if (access(dd->devnode, F_OK) == -1) {
        free(dd->devnode);
        dd->devnode = NULL;
        errno = ENOENT;
        return -1;
    }

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
        *devunit = dd->devunit;
        return 0;
    }

    if (demi_device_get_devname(dd, &devname)) {
        return -1;
    }

    for (i = 0; devname[i] != '\0'; i++) {
        if (devname[i] >= '0' && devname[i] <= '9') {
            dd->devunit = (int32_t)strtol(devname + i, NULL, 10);
            *devunit = dd->devunit;
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

int demi_device_get_class(struct demi_device *dd, enum demi_class *class)
{
    const char *devname;
    dev_t devnum;

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

    if (demi_device_get_devnum(dd, &devnum) == -1) {
        return -1;
    }

    // TODO lookup table
    // TODO cache getdevmajor result (pthread_once?)
    if (major(devnum) == getdevmajor("drm", S_IFCHR)) {
        dd->class = DEMI_CLASS_DRM;
    }
    else if (parse_wscons(devname) != DEMI_TYPE_UNKNOWN) {
        dd->class = DEMI_CLASS_INPUT;
    }
    else {
        dd->class = DEMI_CLASS_UNKNOWN;
    }

    *class = dd->class;
    return 0;
}

int demi_device_get_type(struct demi_device *dd, enum demi_type *type)
{
    const char *devname;

    if (!dd || !type) {
        errno = EINVAL;
        return -1;
    }

    if (dd->type) {
        *type = dd->type;
        return 0;
    } 

    if (demi_device_get_devname(dd, &devname) == -1) {
        return -1;
    }

    dd->type = parse_wscons(devname);
    *type = dd->type;
    return 0;
}

int device_init(struct demi_device *dd, struct demi *ctx,
        const char *devnode, const char *devname, dev_t devnum, mode_t type,
        enum demi_action action)
{
    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
    dd->action = action;
    dd->devnum = devnum;
    dd->devtype = type;
    dd->devunit = -1;

    if (devname) {
        dd->devname = strdup(devname);

        if (!dd->devname) {
            return -1;
        }
    }

    if (devnode) {
        dd->devnode = strdup(devnode);

        if (!dd->devnode) {
            free(dd->devname);
            return -1;
        }
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

    return device_init(dd, ctx, devnode, devnode + 5, 0, 0, 0);
}

int demi_device_init_devnum(struct demi_device *dd, struct demi *ctx,
        dev_t devnum, mode_t type)
{
    if (!dd || !ctx) {
        return -1;
    }

    if (!S_ISCHR(type) && !S_ISBLK(type)) {
        return -1;
    }

    return device_init(dd, ctx, NULL, NULL, devnum, type, 0);
}

void demi_device_finish(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    free(dd->devnode);
    free(dd->devname);
}
