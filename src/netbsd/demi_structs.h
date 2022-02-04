#include <stdint.h>
#include <sys/types.h>

struct demi {
    int fd;
};

struct demi_device {
    struct demi *ctx;
    char *devnode;
    char *devname;

    dev_t devnum;
    mode_t devtype;
    int32_t devunit;

    uint32_t type;
    enum demi_class class;
    enum demi_action action;
};

struct demi_monitor {
    struct demi *ctx;
};

struct demi_enumerate {
    struct demi *ctx;
};
