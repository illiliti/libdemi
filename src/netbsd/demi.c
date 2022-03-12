#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <sys/drvctlio.h>
#include <prop/proplib.h>

#include "demi.h"

int demi_read(int fd, struct demi_event *de)
{
    const char *event, *device;
    prop_dictionary_t dict;
    size_t len;

    if (!de) {
        errno = EINVAL;
        return -1;
    }

    if (prop_dictionary_recv_ioctl(fd, DRVGETEVENT, &dict) != 0) {
        return -1;
    }

    prop_dictionary_get_cstring_nocopy(dict, "event", &event);
    prop_dictionary_get_cstring_nocopy(dict, "device", &device);

    if (strcmp(event, "device-attach") == 0) {
        de->de_type = DEMI_ATTACH;
    }
    else if (strcmp(event, "device-detach") == 0) {
        de->de_type = DEMI_DETACH;
    }
    else {
        de->de_type = DEMI_UNKNOWN;
    }

    len = sizeof(de->de_devname);
    assert(strlcpy(de->de_devname, device, len) < len);
    prop_object_release(dict);
    return 0;
}

int demi_init(int flags)
{
    return open(DRVCTLDEV, O_RDWR | flags);
}
