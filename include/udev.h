#include <libudev.h>

#include "demi.h"

struct demi {
    struct udev *udev;
};

struct demi_device {
    struct udev_device *udev_device;
    struct demi *ctx;

    enum demi_device_action action;
    enum demi_device_class class;
    enum demi_device_type type;
};

struct demi_monitor {
    struct udev_monitor *udev_monitor;
    struct demi *ctx;
};

struct demi_enumerate {
    struct udev_enumerate *udev_enumerate;
    struct demi *ctx;
};

struct demi_device *device_init(struct demi *, struct udev_device *);
