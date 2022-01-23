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
