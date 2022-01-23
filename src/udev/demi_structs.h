#include <libudev.h>

struct demi {
    struct udev *udev;
};

struct demi_device {
    struct udev_device *udev_device;
    struct demi *ctx;

    enum demi_action action;
    enum demi_class class;
    enum demi_type type;
};

struct demi_monitor {
    struct udev_monitor *udev_monitor;
    struct demi *ctx;
};

struct demi_enumerate {
    struct udev_enumerate *udev_enumerate;
    struct demi *ctx;
};
