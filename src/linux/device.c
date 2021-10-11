#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "demi.h"
#include "linux.h"

const char *demi_device_get_devnode(struct demi_device *dd)
{
    size_t len;

    if (!dd || !dd->devname) {
        return NULL;
    }

    if (dd->devnode) {
        return dd->devnode;
    }

    len = 5 + strlen(dd->devname) + 1;
    dd->devnode = malloc(len);

    if (!dd->devnode) {
        return NULL;
    }

    snprintf(dd->devnode, len, "/dev/%s", dd->devname);
    return dd->devnode;
}

const char *demi_device_get_devname(struct demi_device *dd)
{
    return dd ? dd->devname : NULL;
}

dev_t demi_device_get_devnum(struct demi_device *dd)
{
    return dd ? makedev(dd->major, dd->minor) : makedev(0, 0);
}

int demi_device_get_devunit(struct demi_device *dd)
{
    const char *devname;
    int i;

    if (!dd) {
        return -1;
    }

    devname = demi_device_get_devname(dd);

    if (!devname) {
        return -1;
    }

    if (dd->devunit != -1) {
        return dd->devunit;
    }

    for (i = 0; devname[i] != '\0'; i++) {
        if (devname[i] >= '0' && devname[i] <= '9') {
            dd->devunit = (int)strtol(devname + i, NULL, 10);
            return dd->devunit;
        }
    }

    return -1;
}

enum demi_device_action demi_device_get_action(struct demi_device *dd)
{
    if (!dd) {
        return DEMI_ACTION_UNKNOWN;
    }

    if (dd->action) {
        return dd->action;
    }

    dd->action = DEMI_ACTION_UNKNOWN;
    return dd->action;
}

enum demi_device_class demi_device_get_class(struct demi_device *dd)
{
    if (!dd || !dd->subsystem) {
        return DEMI_CLASS_UNKNOWN;
    }

    if (dd->class) {
        return dd->class;
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

    return dd->class;
}

enum demi_device_type demi_device_get_type(struct demi_device *dd)
{
    if (!dd) {
        return DEMI_TYPE_UNKNOWN;
    }

    if (dd->type) {
        return dd->type;
    } 

    if (demi_device_get_class(dd) == DEMI_CLASS_INPUT) {
        dd->type = parse_evdev(&dd->evdev);
    }
    else {
        dd->type = DEMI_TYPE_UNKNOWN;
    }

    return dd->type;
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

static inline int parse_var(struct demi_device *dd, const char *line)
{
    size_t len;

    // /devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/event14
    if (!dd->uevent && strncmp(line, "DEVPATH=", 8) == 0) {
        line += 8;
        len = (strrchr(line, '/') - line) + 1;
        // /sys + .../ + uevent + '\0'
        dd->uevent = malloc(4 + len + 6 + 1);

        if (!dd->uevent) {
            return -1;
        }

        // /sys
        memcpy(dd->uevent, "/sys", 4);
        // /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/
        memcpy(dd->uevent + 4, line, len);
        // /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/0003:093A:2521.0021/input/input47/uevent
        memcpy(dd->uevent + 4 + len, "uevent", 7);
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
        dd->major = (unsigned)strtol(line + 6, NULL, 10);
    }
    else if (strncmp(line, "MINOR=", 6) == 0) {
        dd->minor = (unsigned)strtol(line + 6, NULL, 10);
    }
    else if (strncmp(line, "EV=", 3) == 0) {
        parse_mask(dd->evdev.ev, line + 3);
    }
    else if (strncmp(line, "KEY=", 4) == 0) {
        parse_mask(dd->evdev.key, line + 4);
    }
    else if (strncmp(line, "REL=", 4) == 0) {
        parse_mask(dd->evdev.rel, line + 4);
    }
    else if (strncmp(line, "ABS=", 4) == 0) {
        parse_mask(dd->evdev.abs, line + 4);
    }
    else if (strncmp(line, "PROP=", 5) == 0) {
        parse_mask(dd->evdev.prop, line + 5);
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

static int read_uevent(struct demi_device *dd, int dfd, const char *uevent)
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

        if (parse_var(dd, line) == -1) {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

struct demi_device *device_init_uevent(struct demi *ctx, const char *buf,
        size_t len)
{
    struct demi_device *dd;
    const char *end;

    dd = calloc(1, sizeof(*dd));

    if (!dd) {
        return NULL;
    }

    dd->devunit = -1;

    for (end = buf + len; buf < end; buf += strlen(buf) + 1) {
        if (parse_var(dd, buf) == -1) {
            demi_device_deinit(dd);
            return NULL;
        }
    }

    if (demi_device_get_class(dd) == DEMI_CLASS_INPUT) {
        // TODO document why we can't use fd
        if (read_uevent(dd, -1, dd->uevent) == -1) {
            demi_device_deinit(dd);
            return NULL;
        }
    }

    dd->ctx = ctx;
    return dd;
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

struct demi_device *device_init_syspath(struct demi *ctx, const char *syspath)
{
    struct demi_device *dd;
    int dfd;

    dfd = open(syspath, O_DIRECTORY | O_PATH | O_CLOEXEC);

    if (dfd == -1) {
        return NULL;
    }

    dd = calloc(1, sizeof(*dd));

    if (!dd) {
        close(dfd);
        return NULL;
    }

    dd->devunit = -1;

    if (read_uevent(dd, dfd, "uevent") == -1 ||
        read_subsystem(dd, dfd) == -1) {
        close(dfd);
        demi_device_deinit(dd);
        return NULL;
    }

    switch (demi_device_get_class(dd)) {
    case DEMI_CLASS_DRM:
        if (read_boot_vga(dd, dfd) == -1) {
            close(dfd);
            demi_device_deinit(dd);
            return NULL;
        }

        break;
    case DEMI_CLASS_INPUT:
        if (read_uevent(dd, dfd, "device/uevent") == -1) {
            close(dfd);
            demi_device_deinit(dd);
            return NULL;
        }

        break;
    default:
        break;
    }

    close(dfd);
    dd->ctx = ctx;
    return dd;
}

struct demi_device *demi_device_init_devnum(struct demi *ctx, dev_t devnum,
        mode_t type)
{
    char syspath[48];

    if (!ctx) {
        return NULL;
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
        return NULL;
    }

    return device_init_syspath(ctx, syspath);
}

struct demi_device *demi_device_init_devnode(struct demi *ctx,
        const char *devnode)
{
    struct stat st;

    if (!ctx || !devnode) {
        return NULL;
    }

    if (stat(devnode, &st) == -1) {
        return NULL;
    }

    return demi_device_init_devnum(ctx, st.st_rdev, st.st_mode);
}

void demi_device_deinit(struct demi_device *dd)
{
    if (!dd) {
        return;
    }

    free(dd->subsystem);
    free(dd->devnode);
    free(dd->devname);
    free(dd->uevent);
    free(dd);
}
