#include <sys/types.h>

int device_init(struct demi_device *device, struct demi *context,
        const char *devname, dev_t devnum, mode_t type,
        enum demi_action action);
