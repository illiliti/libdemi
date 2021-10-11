#include "demi.h"
#include "evdev.h"

struct demi {
    char dummy;
};

struct demi_device {
    struct demi *ctx;

    unsigned major, minor;
    int devunit;

    char *subsystem;
    char *devnode;
    char *devname;
    char *uevent;

    struct evdev evdev;

    enum demi_device_action action;
    enum demi_device_class class;
    enum demi_device_type type;
};

struct demi_monitor {
    struct demi *ctx;

    int fd;
};

struct demi_enumerate {
    struct demi *ctx;
};

struct demi_device *device_init_uevent(struct demi *, const char *, size_t);
struct demi_device *device_init_syspath(struct demi *, const char *);
