#include <stdint.h>
#include <sys/types.h>

#define DEMI_DEVNAME_MAX (16 + 1)
#define DEMI_DEVNODE_MAX (DEMI_DEVNAME_MAX + (sizeof("/dev/") - 1))

struct demi {
    int fd;
};

struct demi_device {
    char devnode[DEMI_DEVNODE_MAX];
    char devname[DEMI_DEVNAME_MAX];

    struct demi *ctx;

    dev_t devnum;
    mode_t devtype;
    int32_t devunit;

    enum demi_action action;
};

struct demi_monitor {
    struct demi *ctx;
};

struct demi_enumerate {
    struct demi *ctx;
};
