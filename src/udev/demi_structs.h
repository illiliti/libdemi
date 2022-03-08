#include <stdint.h>
#include <libudev.h>

struct demi {
    struct udev *udev;
};

struct demi_device {
    struct udev_device *udev_device;
    struct demi *ctx;

    uint32_t type;
    enum demi_class class;
    enum demi_action action;
};

struct demi_monitor {
    struct udev_monitor *udev_monitor;
    struct demi *ctx;
};
