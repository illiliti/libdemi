#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>

#define DEMI_DEVNAME_MAX (SPECNAMELEN + 1)
#define DEMI_DEVNODE_MAX (DEMI_DEVNAME_MAX + (sizeof("/dev/") - 1))

struct demi {
    char dummy;
};

struct demi_device {
    char devnode[DEMI_DEVNODE_MAX];
    char devname[DEMI_DEVNAME_MAX];

    struct demi *ctx;

    dev_t devnum;
    mode_t devtype;
    int32_t devunit;

    uint32_t type;
    enum demi_class class;
    enum demi_action action;
};

struct demi_monitor {
    struct demi *ctx;

    int fd;
};
