#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "demi.h"

struct demi {
    size_t ref;

    int fd;
};

struct demi_device {
    struct demi *ctx;
    size_t ref;

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
    size_t ref;
};

struct demi_enumerate {
    struct demi *ctx;
    size_t ref;
};

struct demi_device *device_new(struct demi *, const char *, const char *,
        dev_t, mode_t, enum demi_action);
