#include <stddef.h>
#include <sys/types.h>

int device_init_msg(struct demi_device *device, struct demi *context,
        const char *buf, size_t len);
int device_init(struct demi_device *device, struct demi *context,
        const char *devname, dev_t devnum, mode_t type);
