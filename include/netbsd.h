#include <sys/types.h>

#include "demi.h"

struct demi {
    int fd;
};

struct demi_device {
    struct demi *ctx;

    mode_t devtype;
    dev_t devnum;
    int devunit;

    char *devnode;
    char *devname;

    enum demi_device_action action;
    enum demi_device_class class;
    enum demi_device_type type;
};

struct demi_monitor {
    struct demi *ctx;
};

struct demi_enumerate {
    struct demi *ctx;
};

struct demi_device *device_init(struct demi *, const char *, const char *,
        dev_t, mode_t, enum demi_device_action);
