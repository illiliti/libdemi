#include <stdint.h>
#include <stddef.h>

#include "demi.h"
#include "evdev.h"

struct demi {
    size_t ref;
};

struct demi_device {
    struct demi *ctx;
    size_t ref;

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
    size_t ref;

    int fd;
};

struct demi_enumerate {
    struct demi *ctx;
    size_t ref;
};

struct demi_device *device_new_uevent(struct demi *, const char *, size_t);
struct demi_device *device_new_syspath(struct demi *, int, const char *);
