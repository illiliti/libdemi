#include <stdint.h>
#include <sys/types.h>

#include "demi.h"

struct demi {
    int fd;
};

struct demi_device {
    struct demi *ctx;

    dev_t devnum;
    mode_t devtype;
    int32_t devunit;

    char *devnode;
    char *devname;

    enum demi_action action;
    enum demi_class class;
    enum demi_type type;
};

struct demi_monitor {
    struct demi *ctx;
};

struct demi_enumerate {
    struct demi *ctx;
};

struct demi_device *device_init(struct demi *, const char *, const char *,
        dev_t, mode_t, enum demi_action);
