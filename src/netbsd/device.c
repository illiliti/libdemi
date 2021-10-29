#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "demi.h"
#include "netbsd.h"
#include "wscons.h"

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    size_t len = 1;

    if (!dd || !devname) {
        return -EINVAL;
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
            break;
        case ERANGE:
            continue;
        case ENOENT:
            free(dd->devname);
            dd->devname = NULL;
            return -ENOENT;
        default:
            abort();
        }
    }

    *devname = dd->devname;
    return 0;
}

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    const char *devname;
    size_t len;
    int ret;

    if (!dd || !devnode) {
        return -EINVAL;
    }

    if (dd->devnode) {
        *devnode = dd->devnode;
        return 0;
    }

    ret = demi_device_get_devname(dd, &devname);

    if (ret < 0) {
        return ret;
    }

    len = 5 + strlen(devname) + 1;
    dd->devnode = malloc(len);

    if (!dd->devnode) {
        return -1;
    }

    // TODO use memcpy
    snprintf(dd->devnode, len, "/dev/%s", devname);

    // FIXME find a better way to check devnode
    if (access(dd->devnode, F_OK) == -1) {
        free(dd->devnode);
        dd->devnode = NULL;
        return -ENOENT;
    }

    *devnode = dd->devnode;
    return 0;
}

int demi_device_get_devnum(struct demi_device *dd, dev_t *devnum)
{
    const char *devnode;
    struct stat st;
    int ret;

    if (!dd || !devnum) {
        return -EINVAL;
    }

    if (dd->devnum) {
        *devnum = dd->devnum;
        return 0;
    }

    ret = demi_device_get_devnode(dd, &devnode);

    if (ret < 0) {
        return ret;
    }

    if (stat(devnode, &st) == -1) {
        return -ENOENT;
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
    int i, ret;

    if (!dd || !devunit) {
        return -1;
    }

    if (dd->devunit != -1) {
        *devunit = dd->devunit;
        return 0;
    }

    ret = demi_device_get_devname(dd, &devname);

    if (ret < 0) {
        return ret;
    }

    for (i = 0; devname[i] != '\0'; i++) {
        if (devname[i] >= '0' && devname[i] <= '9') {
            dd->devunit = strtol(devname + i, NULL, 10);
            *devunit = dd->devunit;
            return 0;
        }
    }

    return -ENOENT;
}

int demi_device_get_seat(struct demi_device *dd, const char **seat)
{
    if (!dd || !seat) {
        return -EINVAL;
    }

    return -ENOTSUP;
}

int demi_device_get_action(struct demi_device *dd, enum demi_action *action)
{
    if (!dd || !action) {
        return -EINVAL;
    }

    if (!dd->action) {
        return -ENOENT;
    }

    *action = dd->action;
    return 0;
}

int demi_device_get_class(struct demi_device *dd, enum demi_class *class)
{
    const char *devname;
    dev_t devnum;
    int ret;

    if (!dd || !class) {
        return -EINVAL;
    }

    if (dd->class) {
        *class = dd->class;
        return 0;
    } 

    ret = demi_device_get_devname(dd, &devname);

    if (ret < 0) {
        return ret;
    }

    ret = demi_device_get_devnum(dd, &devnum);

    if (ret < 0) {
        return ret;
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
    int ret;

    if (!dd || !type) {
        return -EINVAL;
    }

    if (dd->type) {
        *type = dd->type;
        return 0;
    } 

    ret = demi_device_get_devname(dd, &devname);

    if (ret < 0) {
        return ret;
    }

    dd->type = parse_wscons(devname);
    *type = dd->type;
    return 0;
}

struct demi_device *device_init(struct demi *ctx, const char *devnode,
        const char *devname, dev_t devnum, mode_t type,
        enum demi_action action)
{
    struct demi_device *dd;

    dd = calloc(1, sizeof(*dd));

    if (!dd) {
        return NULL;
    }

    dd->action = action;
    dd->devnum = devnum;
    dd->devtype = type;
    dd->devunit = -1;

    if (devname) {
        dd->devname = strdup(devname);

        if (!dd->devname) {
            free(dd);
            return NULL;
        }
    }

    if (devnode) {
        dd->devnode = strdup(devnode);

        if (!dd->devnode) {
            free(dd->devname);
            free(dd);
            return NULL;
        }
    }

    dd->ctx = ctx;
    return dd;
}

struct demi_device *demi_device_init_devnode(struct demi *ctx,
        const char *devnode)
{
    if (!ctx || !devnode) {
        return NULL;
    }

    if (strncmp(devnode, "/dev/", 5) != 0) {
        return NULL;
    }

    return device_init(ctx, devnode, devnode + 5, 0, 0, 0);
}

struct demi_device *demi_device_init_devnum(struct demi *ctx, dev_t devnum,
        mode_t type)
{
    if (!ctx) {
        return NULL;
    }

    if (!S_ISCHR(type) && !S_ISBLK(type)) {
        return NULL;
    }

    return device_init(ctx, NULL, NULL, devnum, type, 0);
}

void demi_device_deinit(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    free(dd->devnode);
    free(dd->devname);
    free(dd);
}
