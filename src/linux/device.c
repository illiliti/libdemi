#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "demi.h"
#include "device.h"

int demi_device_get_devnode(struct demi_device *dd, const char **devnode)
{
    if (!dd || !devnode) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devnode[0] != '\0') {
        *devnode = dd->devnode;
        return 0;
    }

    if (dd->devname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    snprintf(dd->devnode, sizeof(dd->devnode), "/dev/%s", dd->devname);
    *devnode = dd->devnode;
    return 0;
}

int demi_device_get_devname(struct demi_device *dd, const char **devname)
{
    if (!dd || !devname) {
        errno = EINVAL;
        return -1;
    }

    if (dd->devname[0] == '\0') {
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

    *devnum = makedev((uint32_t)dd->major, (uint32_t)dd->minor);
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
        *devunit = (uint32_t)dd->devunit;
        return 0;
    }

    if (dd->devname[0] == '\0') {
        errno = ENOENT;
        return -1;
    }

    for (i = 0; dd->devname[i] != '\0'; i++) {
        if (dd->devname[i] >= '0' && dd->devname[i] <= '9') {
            dd->devunit = (int32_t)strtol(dd->devname + i, NULL, 10);
            *devunit = (uint32_t)dd->devunit;
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

static inline void parse_var(struct demi_device *dd, const char *line)
{
    const char *value;

    value = strchr(line, '=');

    if (!value) {
        return;
    }

    value += 1; // strip leading '='

    if (dd->devname[0] == '\0' && strncmp(line, "DEVNAME=", 8) == 0) {
        snprintf(dd->devname, sizeof(dd->devname), "%s", value);
    }
    else if (strncmp(line, "MAJOR=", 6) == 0) {
        dd->major = (int32_t)strtol(value, NULL, 10);
    }
    else if (strncmp(line, "MINOR=", 6) == 0) {
        dd->minor = (int32_t)strtol(value, NULL, 10);
    }
    else if (strncmp(line, "ACTION=", 7) != 0) {
        return;
    }
    else if (strcmp(value, "add") == 0) {
        dd->action = DEMI_ACTION_ATTACH;
    }
    else if (strcmp(value, "remove") == 0) {
        dd->action = DEMI_ACTION_DETACH;
    }
    else if (strcmp(value, "change") == 0) {
        dd->action = DEMI_ACTION_CHANGE;
    }
    else {
        dd->action = DEMI_ACTION_UNKNOWN;
    }
}

static int read_uevent(struct demi_device *dd, int dfd)
{
    char line[LINE_MAX];
    FILE *fp;
    int fd;

    fd = openat(dfd, "uevent", O_RDONLY | O_CLOEXEC);

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
        parse_var(dd, line);
    }

    fclose(fp);
    return 0;
}

int device_init_uevent(struct demi_device *dd, struct demi *ctx,
        const char *buf, size_t len)
{
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
        parse_var(dd, buf);
    }

    return 0;
}

int device_init_syspath(struct demi_device *dd, struct demi *ctx, int bfd,
        const char *syspath)
{
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

    if (read_uevent(dd, dfd) == -1) {
        close(dfd);
        return -1;
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

    switch (type) {
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

    st.st_mode = st.st_mode & (S_IFBLK | S_IFCHR);
    return demi_device_init_devnum(dd, ctx, st.st_rdev, st.st_mode);
}

void demi_device_finish(struct demi_device *dd)
{
    (void)dd;
}
