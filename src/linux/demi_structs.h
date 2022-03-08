#include <stdint.h>

#define DEMI_DEVNAME_MAX (255 + 1)
#define DEMI_DEVNODE_MAX (DEMI_DEVNAME_MAX + (sizeof("/dev/") - 1))

struct demi {
    char dummy;
};

struct demi_device {
    char devnode[DEMI_DEVNODE_MAX];
    char devname[DEMI_DEVNAME_MAX];

    struct demi *ctx;

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
