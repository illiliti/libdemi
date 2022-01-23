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
