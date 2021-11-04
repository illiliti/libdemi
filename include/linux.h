#include <stdint.h>
#include <stddef.h>

#include "demi.h"
#include "evdev.h"

struct demi {
    char dummy;
};

struct demi_device {
    struct demi *ctx;

    int32_t major;
    int32_t minor;
    int32_t devunit;

    char *parent_uevent;

    char *subsystem;
    char *devnode;
    char *devname;

    struct evdev evdev;

    enum demi_action action;
    enum demi_class class;
    enum demi_type type;
};

struct demi_monitor {
    struct demi *ctx;

    int fd;
};

struct demi_enumerate {
    struct demi *ctx;
};

struct demi_device *device_init_uevent(struct demi *, const char *, size_t);
struct demi_device *device_init_syspath(struct demi *, int, const char *);
