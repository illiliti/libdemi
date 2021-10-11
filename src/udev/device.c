#include <string.h>
#include <stdlib.h>
#include <libudev.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "demi.h"
#include "udev.h"

static const struct {
    const char *prop;
    enum demi_device_type type;
} prop_type[] = {
    { "ID_INPUT_MOUSE", DEMI_TYPE_MOUSE },
    { "ID_INPUT_TABLET", DEMI_TYPE_TABLET },
    { "ID_INPUT_TOUCHPAD", DEMI_TYPE_TOUCHPAD },
    { "ID_INPUT_KEYBOARD", DEMI_TYPE_KEYBOARD },
    { "ID_INPUT_JOYSTICK", DEMI_TYPE_JOYSTICK },
    { "ID_INPUT_TOUCHSCREEN", DEMI_TYPE_TOUCHSCREEN },
    { "ID_INPUT_SWITCH", DEMI_TYPE_SWITCH },
    { "ID_INPUT_POINTINGSTICK", DEMI_TYPE_POINTING_STICK },
    { "ID_INPUT_ACCELEROMETER", DEMI_TYPE_ACCELEROMETER },
    { NULL, DEMI_TYPE_UNKNOWN },
};

const char *demi_device_get_devnode(struct demi_device *dd)
{
    return dd ? udev_device_get_devnode(dd->udev_device) : NULL;
}

const char *demi_device_get_devname(struct demi_device *dd)
{
    return dd ? udev_device_get_sysname(dd->udev_device) : NULL;
}

dev_t demi_device_get_devnum(struct demi_device *dd)
{
    return dd ? udev_device_get_devnum(dd->udev_device) : makedev(0, 0);
}

int demi_device_get_devunit(struct demi_device *dd)
{
    const char *sysnum;
    
    if (!dd) {
        return -1;
    }

    sysnum = udev_device_get_sysnum(dd->udev_device);

    if (!sysnum) {
        return -1;
    }

    return (int)strtol(sysnum, NULL, 10);
}

enum demi_device_action demi_device_get_action(struct demi_device *dd)
{
    const char *action;

    if (!dd) {
        return DEMI_ACTION_UNKNOWN;
    }

    if (dd->action) {
        return dd->action;
    }

    action = udev_device_get_action(dd->udev_device);

    // TODO lookup table
    if (!action) {
        dd->action = DEMI_ACTION_UNKNOWN;
    }
    else if (strcmp(action, "add") == 0) {
        dd->action = DEMI_ACTION_ATTACH;
    }
    else if (strcmp(action, "remove") == 0) {
        dd->action = DEMI_ACTION_DETACH;
    }
    else if (strcmp(action, "change") == 0) {
        dd->action = DEMI_ACTION_CHANGE;
    }
    else {
        dd->action = DEMI_ACTION_UNKNOWN;
    }

    return dd->action;
}

enum demi_device_class demi_device_get_class(struct demi_device *dd)
{
    const char *subsystem;

    if (!dd) {
        return DEMI_CLASS_UNKNOWN;
    }

    if (dd->class) {
        return dd->class;
    } 

    subsystem = udev_device_get_subsystem(dd->udev_device);

    // TODO lookup table
    if (!subsystem) {
        dd->class = DEMI_CLASS_UNKNOWN;
    }
    else if (strcmp(subsystem, "drm") == 0) {
        dd->class = DEMI_CLASS_DRM;
    }
    else if (strcmp(subsystem, "input") == 0) {
        dd->class = DEMI_CLASS_INPUT;
    }
    else {
        dd->class = DEMI_CLASS_UNKNOWN;
    }

    return dd->class;
}

enum demi_device_type demi_device_get_type(struct demi_device *dd)
{
    struct udev_device *parent;
    const char *boot_vga;
    int i;
    
    if (!dd) {
        return DEMI_TYPE_UNKNOWN;
    }

    if (dd->type) {
        return dd->type;
    } 

    dd->type = DEMI_TYPE_UNKNOWN;

    switch (demi_device_get_class(dd)) {
    case DEMI_CLASS_DRM:
        parent = udev_device_get_parent_with_subsystem_devtype(dd->udev_device,
                "pci", NULL);

        if (parent) {
            boot_vga = udev_device_get_sysattr_value(parent, "boot_vga");

            if (boot_vga && boot_vga[0] == '1') {
                dd->type = DEMI_TYPE_BOOT_VGA;
            }
        }

        break;
    case DEMI_CLASS_INPUT:
        for (i = 0; prop_type[i].prop; i++) {
            if (udev_device_get_property_value(dd->udev_device,
                prop_type[i].prop)) {
                dd->type = prop_type[i].type;
                break;
            }
        }

        break;
    default:
        break;
    }

    return dd->type;
}

struct demi_device *device_init(struct demi *ctx,
        struct udev_device *udev_device)
{
    struct demi_device *dd;

    dd = calloc(1, sizeof(*dd));

    if (!dd) {
        return NULL;
    }

    dd->udev_device = udev_device;
    dd->ctx = ctx;
    return dd;
}

struct demi_device *demi_device_init_devnum(struct demi *ctx, dev_t devnum,
        mode_t type)
{
    struct udev_device *udev_device;
    struct demi_device *dd;

    if (!ctx) {
        return NULL;
    }

    switch (type & (S_IFCHR | S_IFBLK)) {
    case S_IFCHR:
        udev_device = udev_device_new_from_devnum(ctx->udev, 'c', devnum);
        break;
    case S_IFBLK:
        udev_device = udev_device_new_from_devnum(ctx->udev, 'b', devnum);
        break;
    default:
        return NULL;
    }

    if (!udev_device) {
        return NULL;
    }

    dd = device_init(ctx, udev_device);

    if (!dd) {
        udev_device_unref(udev_device);
        return NULL;
    }

    return dd;
}

struct demi_device *demi_device_init_devnode(struct demi *ctx,
        const char *devnode)
{
    struct stat st;

    if (!ctx || !devnode) {
        return NULL;
    }

    if (stat(devnode, &st) == -1) {
        return NULL;
    }

    return demi_device_init_devnum(ctx, st.st_rdev, st.st_mode);
}

void demi_device_deinit(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    udev_device_unref(dd->udev_device);
    free(dd);
}
