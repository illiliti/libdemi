#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "demi.h"

int demi_read(int fd, struct demi_event *de)
{
    struct sockaddr_nl sa = {0};
    struct msghdr hdr = {0};
    struct iovec iov = {0};

    char buf[8192], *msg, *end;
    char *ptr, *key, *value;
    ssize_t len;

    if (!de) {
        errno = EINVAL;
        return -1;
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    hdr.msg_name = &sa;
    hdr.msg_namelen = sizeof(sa);
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    len = recvmsg(fd, &hdr, 0);

    if (len <= 0) {
        return -1;
    }

    if (hdr.msg_flags & MSG_TRUNC) {
        return -1;
    }

    if (sa.nl_groups == 0x0 || (sa.nl_groups == 0x1 && sa.nl_pid != 0)) {
        return -1;
    }

    len -= 1;
    assert(buf[len] == '\0');
    msg = buf;
    *de = (struct demi_event){0};

    for (end = msg + len; msg < end; msg += strlen(msg) + 1) {
        key = strtok_r(msg, "=", &ptr);
        value = strtok_r(NULL, "=", &ptr);

        if (!key || !value) {
            continue;
        }

        if (strcmp(key, "DEVNAME") == 0) {
            assert(strlen(value) < sizeof(de->de_devname));
            snprintf(de->de_devname, sizeof(de->de_devname), "%s", value);
        }
        else if (strcmp(key, "ACTION") != 0) {
            continue;
        }
        else if (strcmp(value, "add") == 0) {
            de->de_type = DEMI_ATTACH;
        }
        else if (strcmp(value, "remove") == 0) {
            de->de_type = DEMI_DETACH;
        }
        else if (strcmp(value, "change") == 0) {
            de->de_type = DEMI_CHANGE;
        }
        else {
            de->de_type = DEMI_UNKNOWN;
        }
    }

    return 0;
}

int demi_init(int flags)
{
    struct sockaddr_nl sa = {0};
    int fd;

    fd = socket(AF_NETLINK, SOCK_RAW | flags, NETLINK_KOBJECT_UEVENT);

    if (fd == -1) {
        return -1;
    }

    sa.nl_family = AF_NETLINK;
    sa.nl_groups = DEMI_MONITOR_NETLINK_GROUP;

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}
