#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/device.h>
#include <sys/hotplug.h>

#include "demi.h"

int demi_read(int fd, struct demi_event *de)
{
    struct hotplug_event he;
    size_t len;

    if (!de) {
        errno = EINVAL;
        return -1;
    }

    if (read(fd, &he, sizeof(he)) <= 0) {
        return -1;
    }

    switch (he.he_type) {
    case HOTPLUG_DEVAT:
        de.de_type = DEMI_ATTACH;
        break;
    case HOTPLUG_DEVDT:
        de.de_type = DEMI_DETACH;
        break;
    default:
        de.de_type = DEMI_UNKNOWN;
        break;
    }

    len = sizeof(de.de_devname);
    assert(strlcpy(de.de_devname, he.he_devname, len) < len);
    return 0;
}

int demi_init(int flags)
{
    return open("/dev/hotplug", O_RDONLY | flags);
}
