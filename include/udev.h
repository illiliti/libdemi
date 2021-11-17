#include <stddef.h>
#include <libudev.h>

#include "demi.h"

struct demi {
    struct udev *udev;
    size_t ref;
};

struct demi_device {
    struct udev_device *udev_device;
    struct demi *ctx;
    size_t ref;

    enum demi_action action;
    enum demi_class class;
    enum demi_type type;
};

struct demi_monitor {
    struct udev_monitor *udev_monitor;
    struct demi *ctx;
    size_t ref;
};

struct demi_enumerate {
    struct udev_enumerate *udev_enumerate;
    struct demi *ctx;
    size_t ref;
};

struct demi_device *device_new(struct demi *, struct udev_device *);
