#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "demi.h"
#include "netbsd.h"
#include "wscons.h"

const char *demi_device_get_devname(struct demi_device *dd)
{
    char *devname;
    size_t len;

    if (!dd) {
        return NULL;
    }

    if (dd->devname) {
        return dd->devname;
    }

    len = 48 + 1;
    dd->devname = malloc(len);

    if (!dd->devname) {
        return NULL;
    }

    while (devname_r(dd->devnum, dd->devtype, dd->devname, len) != 0) {
        if (errno != ERANGE) {
            free(dd->devname);
            dd->devname = NULL;
            return NULL;
        }

        devname = realloc(dd->devname, (len *= 2));

        if (!devname) {
            free(dd->devname);
            dd->devname = NULL;
            return NULL;
        }

        dd->devname = devname;
    }

    return dd->devname;
}

const char *demi_device_get_devnode(struct demi_device *dd)
{
    const char *devname;
    size_t len;

    if (!dd) {
        return NULL;
    }

    if (dd->devnode) {
        return dd->devnode;
    }

    devname = demi_device_get_devname(dd);

    if (!devname) {
        return NULL;
    }

    len = 5 + strlen(devname) + 1;
    dd->devnode = malloc(len);

    if (!dd->devnode) {
        return NULL;
    }

    snprintf(dd->devnode, len, "/dev/%s", devname);

    if (access(dd->devnode, F_OK) == -1) {
        free(dd->devnode);
        dd->devnode = NULL;
        return NULL;
    }

    return dd->devnode;
}

dev_t demi_device_get_devnum(struct demi_device *dd)
{
    const char *devnode;
    struct stat st;

    if (!dd) {
        return makedev(0, 0);
    }

    if (dd->devnum) {
        return dd->devnum;
    }

    devnode = demi_device_get_devnode(dd);

    if (!devnode) {
        return makedev(0, 0);
    }

    if (stat(devnode, &st) == -1) {
        return makedev(0, 0);
    }

    dd->devtype = st.st_mode;
    dd->devnum = st.st_rdev;
    return dd->devnum;
}

// TODO use DRVCTLCOMMAND to get unit
int demi_device_get_devunit(struct demi_device *dd)
{
    const char *devname;
    int i;

    if (!dd) {
        return -1;
    }

    devname = demi_device_get_devname(dd);

    if (!devname) {
        return -1;
    }

    if (dd->devunit != -1) {
        return dd->devunit;
    }

    for (i = 0; devname[i] != '\0'; i++) {
        if (devname[i] >= '0' && devname[i] <= '9') {
            dd->devunit = (int)strtol(devname + i, NULL, 10);
            return dd->devunit;
        }
    }

    return -1;
}

enum demi_device_action demi_device_get_action(struct demi_device *dd)
{
    if (!dd) {
        return DEMI_ACTION_UNKNOWN;
    }

    if (dd->action) {
        return dd->action;
    }

    dd->action = DEMI_ACTION_UNKNOWN;
    return dd->action;
}

enum demi_device_class demi_device_get_class(struct demi_device *dd)
{
    const char *devname;

    if (!dd) {
        return DEMI_CLASS_UNKNOWN;
    }

    if (dd->class) {
        return dd->class;
    } 

    devname = demi_device_get_devname(dd);

    // TODO lookup table
    if (!devname) {
        dd->class = DEMI_CLASS_UNKNOWN;
    }
    else if (major(demi_device_get_devnum(dd)) == getdevmajor("drm", S_IFCHR)) {
        dd->class = DEMI_CLASS_DRM;
    }
    else if (parse_wscons(devname) != DEMI_TYPE_UNKNOWN) {
        dd->class = DEMI_CLASS_INPUT;
    }

    return dd->class;
}

enum demi_device_type demi_device_get_type(struct demi_device *dd)
{
    const char *devname;

    if (!dd) {
        return DEMI_TYPE_UNKNOWN;
    }

    if (dd->type) {
        return dd->type;
    } 

    devname = demi_device_get_devname(dd);

    if (!devname) {
        dd->type = DEMI_TYPE_UNKNOWN;
    }
    else {
        dd->type = parse_wscons(devname);
    }

    return dd->type;
}

struct demi_device *device_init(struct demi *ctx, const char *devnode,
        const char *devname, dev_t devnum, mode_t type,
        enum demi_device_action action)
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

    return device_init(ctx, devnode, devnode + 5, 0, 0, DEMI_ACTION_UNKNOWN);
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

    return device_init(ctx, NULL, NULL, devnum, type, DEMI_ACTION_UNKNOWN);
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
