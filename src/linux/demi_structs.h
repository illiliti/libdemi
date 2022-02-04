#include <stdint.h>

struct demi {
    char dummy;
};

struct demi_device {
    struct demi *ctx;
    char *parent_uevent;
    char *subsystem;
    char *devnode;
    char *devname;

    int32_t major;
    int32_t minor;
    int32_t devunit;

    uint32_t type;
    enum demi_class class;
    enum demi_action action;
};

struct demi_monitor {
    struct demi *ctx;

    int fd;
};

struct demi_enumerate {
    struct demi *ctx;
};
