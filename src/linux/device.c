#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "demi.h"
#include "evdev.h"
#include "device.h"

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    size_t len;

    if (!dd || !devnode) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devnode) {
        *devnode = dd->devnode;
        return 0;
    }

    if (!dd->devname) {
        errno = ENOENT;
        return -1;
    }

    len = 5 + strlen(dd->devname) + 1;
    dd->devnode = malloc(len);

    if (!dd->devnode) {
        return -1;
    }

    // TODO use memcpy
    snprintf(dd->devnode, len, "/dev/%s", dd->devname);
    *devnode = dd->devnode;
    return 0;
}

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    if (!dd->devname) {
        errno = ENOENT;
        return -1;
    }

    *devname = dd->devname;
    return 0;
}

int demi_device_get_devnum(struct demi_device *dd, dev_t *devnum)
{
    if (!dd || !devnum) {
        errno = EINVAL;
        return -1;
    }

    if (dd->major == -1 || dd->minor == -1) {
        errno = ENOENT;
        return -1;
    }

    *devnum = makedev(dd->major, dd->minor);
    return 0;
}

int demi_device_get_devunit(struct demi_device *dd, uint32_t *devunit)
{
    size_t i;

    if (!dd || !devunit) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devunit != -1) {
        *devunit = dd->devunit;
        return 0;
    }

    if (!dd->devname) {
        errno = ENOENT;
        return -1;
    }

    for (i = 0; dd->devname[i] != '\0'; i++) {
        if (dd->devname[i] >= '0' && dd->devname[i] <= '9') {
            dd->devunit = (int32_t)strtol(dd->devname + i, NULL, 10);
            *devunit = dd->devunit;
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

int demi_device_get_seat(struct demi_device *dd, const char **seat)
{
    if (!dd || !seat) {
        errno = EINVAL;
        return -1;
    }

    errno = ENOTSUP;
    return -1;
}

int demi_device_get_action(struct demi_device *dd, enum demi_action *action)
{
    if (!dd || !action) {
        errno = EINVAL;
        return -1;
    }

    if (!dd->action) {
        errno = ENOENT;
        return -1;
    }

    *action = dd->action;
    return 0;
}

int demi_device_get_class(struct demi_device *dd, enum demi_class *class)
{
    if (!dd || !class) {
        errno = EINVAL;
        return -1;
    }

    if (dd->class) {
        *class = dd->class;
        return 0;
    }

    if (!dd->subsystem) {
        errno = ENOENT;
        return -1;
    }

    // TODO lookup table
    if (strcmp(dd->subsystem, "drm") == 0) {
        dd->class = DEMI_CLASS_DRM;
    }
    else if (strcmp(dd->subsystem, "input") == 0) {
        dd->class = DEMI_CLASS_INPUT;
    }
    else {
        dd->class = DEMI_CLASS_UNKNOWN;
    }

    *class = dd->class;
    return 0;
}

int demi_device_get_type(struct demi_device *dd, enum demi_type *type)
{
    if (!dd || !type) {
        errno = EINVAL;
        return -1;
    }

    if (dd->type) {
        *type = dd->type;
        return 0;
    } 

    *type = DEMI_TYPE_UNKNOWN;
    return 0;
}

static inline void parse_mask(unsigned long *arr, const char *str)
{
    size_t len, i;

    for (i = 0, len = strlen(str); len > 0; len--) {
        if (str[len] == ' ') {
            arr[i++] = strtoul(str + len + 1, NULL, 16);
        }
    }

    arr[i] = strtoul(str, NULL, 16);
}

static inline int parse_var(struct demi_device *dd, struct evdev *evdev,
        const char *line)
{
    size_t len;

    // DEVPATH=/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/event14
    if (!dd->parent_uevent && strncmp(line, "DEVPATH=", 8) == 0) {
        line += 8;
        len = (strrchr(line, '/') - line) + 1;
        // /sys + /.../ + uevent + '\0'
        dd->parent_uevent = malloc(4 + len + 6 + 1);

        if (!dd->parent_uevent) {
            return -1;
        }

        // XXX
        // snprintf(dd->parent_uevent, 4 + len + 6 + 1, "/sys/%.*s/uevent", (int)len, line);

        // /sys
        memcpy(dd->parent_uevent, "/sys", 4);
        // /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/
        memcpy(dd->parent_uevent + 4, line, len);
        // /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/uevent
        memcpy(dd->parent_uevent + 4 + len, "uevent", 7);
    }
    else if (!dd->devname && strncmp(line, "DEVNAME=", 8) == 0) {
        dd->devname = strdup(line + 8);
        return dd->devname ? 0 : -1;
    }
    else if (!dd->subsystem && strncmp(line, "SUBSYSTEM=", 10) == 0) {
        dd->subsystem = strdup(line + 10);
        return dd->subsystem ? 0 : -1;
    }
    else if (strncmp(line, "MAJOR=", 6) == 0) {
        dd->major = (int32_t)strtol(line + 6, NULL, 10);
    }
    else if (strncmp(line, "MINOR=", 6) == 0) {
        dd->minor = (int32_t)strtol(line + 6, NULL, 10);
    }
    else if (strncmp(line, "EV=", 3) == 0) {
        parse_mask(evdev->ev, line + 3);
    }
    else if (strncmp(line, "KEY=", 4) == 0) {
        parse_mask(evdev->key, line + 4);
    }
    else if (strncmp(line, "REL=", 4) == 0) {
        parse_mask(evdev->rel, line + 4);
    }
    else if (strncmp(line, "ABS=", 4) == 0) {
        parse_mask(evdev->abs, line + 4);
    }
    else if (strncmp(line, "PROP=", 5) == 0) {
        parse_mask(evdev->prop, line + 5);
    }
    else if (strncmp(line, "ACTION=", 7) != 0) {
        return 0;
    }
    else if (strcmp(line + 7, "add") == 0) {
        dd->action = DEMI_ACTION_ATTACH;
    }
    else if (strcmp(line + 7, "remove") == 0) {
        dd->action = DEMI_ACTION_DETACH;
    }
    else if (strcmp(line + 7, "change") == 0) {
        dd->action = DEMI_ACTION_CHANGE;
    }
    else {
        dd->action = DEMI_ACTION_UNKNOWN;
    }

    return 0;
}

static int read_uevent(struct demi_device *dd, struct evdev *evdev, int dfd,
        const char *uevent)
{
    char line[LINE_MAX];
    FILE *fp;
    int fd;

    fd = openat(dfd, uevent, O_RDONLY | O_CLOEXEC);

    if (fd == -1) {
        return 0;
    }

    fp = fdopen(fd, "r");

    if (!fp) {
        close(fd);
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        line[strlen(line) - 1] = '\0';

        if (parse_var(dd, evdev, line) == -1) {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

int device_init_uevent(struct demi_device *dd, struct demi *ctx,
        const char *buf, size_t len)
{
    struct evdev evdev = {0};
    enum demi_class class;
    const char *end;

    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
    dd->major = -1;
    dd->minor = -1;
    dd->devunit = -1;

    // add@/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:093A:2521.0002/input/input16\0
    // ACTION=add\0
    // DEVPATH=/devices/pci0000:00/0000:00:14.0/usb1/1-1/1-1:1.0/0003:093A:2521.0002/input/input16\0
    // SUBSYSTEM=input\0
    // ...
    for (end = buf + len; buf < end; buf += strlen(buf) + 1) {
        if (parse_var(dd, &evdev, buf) == -1) {
            return -1;
        }
    }

    if (demi_device_get_class(dd, &class) == 0 && class == DEMI_CLASS_INPUT) {
        // TODO document why we can't use fd
        if (read_uevent(dd, &evdev, -1, dd->parent_uevent) == -1) {
            return -1;
        }

        dd->type = parse_evdev(&evdev);
    }

    return 0;
}

static int read_subsystem(struct demi_device *dd, int dfd)
{
    const char *subsystem;
    char path[48];
    ssize_t len;

    len = readlinkat(dfd, "subsystem", path, sizeof(path));

    if (len == -1) {
        return 0;
    }

    path[len] = '\0';
    subsystem = strrchr(path, '/');

    if (!subsystem) {
        return -1;
    }

    dd->subsystem = strdup(subsystem + 1);
    return dd->subsystem ? 0 : -1;
}

static int read_boot_vga(struct demi_device *dd, int dfd)
{
    char boot_vga;
    int fd;

    fd = openat(dfd, "device/boot_vga", O_RDONLY | O_CLOEXEC);

    if (fd == -1) {
        return 0;
    }

    if (read(fd, &boot_vga, sizeof(boot_vga)) == -1) {
        close(fd);
        return -1;
    }

    if (boot_vga == '1') {
        dd->type = DEMI_TYPE_BOOT_VGA;
    }

    close(fd);
    return 0;
}

int device_init_syspath(struct demi_device *dd, struct demi *ctx, int bfd,
        const char *syspath)
{
    struct evdev evdev = {0};
    enum demi_class class;
    int dfd;

    dfd = openat(bfd, syspath, O_DIRECTORY | O_PATH | O_CLOEXEC);

    if (dfd == -1) {
        return -1;
    }

    memset(dd, 0, sizeof(*dd));

    dd->ctx = ctx;
    dd->major = -1;
    dd->minor = -1;
    dd->devunit = -1;

    if (read_uevent(dd, &evdev, dfd, "uevent") == -1 ||
        read_subsystem(dd, dfd) == -1) {
        close(dfd);
        return -1;
    }

    if (demi_device_get_class(dd, &class) == 0) {
        switch (class) {
        case DEMI_CLASS_DRM:
            if (read_boot_vga(dd, dfd) == -1) {
                close(dfd);
                return -1;
            }

            break;
        case DEMI_CLASS_INPUT:
            if (read_uevent(dd, &evdev, dfd, "device/uevent") == -1) {
                close(dfd);
                return -1;
            }

            dd->type = parse_evdev(&evdev);
            break;
        default:
            break;
        }
    }

    close(dfd);
    return 0;
}

int demi_device_init_devnum(struct demi_device *dd, struct demi *ctx,
        dev_t devnum, mode_t type)
{
    char syspath[48];

    if (!dd || !ctx) {
        return -1;
    }

    switch (type & (S_IFCHR | S_IFBLK)) {
    case S_IFCHR:
        snprintf(syspath, sizeof(syspath), "/sys/dev/char/%u:%u",
                major(devnum), minor(devnum));
        break;
    case S_IFBLK:
        snprintf(syspath, sizeof(syspath), "/sys/dev/block/%u:%u",
                major(devnum), minor(devnum));
        break;
    default:
        return -1;
    }

    return device_init_syspath(dd, ctx, -1, syspath);
}

int demi_device_init_devnode(struct demi_device *dd, struct demi *ctx,
        const char *devnode)
{
    struct stat st;

    if (!dd || !ctx || !devnode) {
        return -1;
    }

    if (stat(devnode, &st) == -1) {
        return -1;
    }

    return demi_device_init_devnum(dd, ctx, st.st_rdev, st.st_mode);
}

void demi_device_finish(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    free(dd->parent_uevent);
    free(dd->subsystem);
    free(dd->devnode);
    free(dd->devname);
}
