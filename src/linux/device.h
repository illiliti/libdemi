#include <stddef.h>

int device_init_uevent(struct demi_device *device, struct demi *context,
        const char *buf, size_t len);
int device_init_syspath(struct demi_device *device, struct demi *context,
        int dfd, const char *syspath);
