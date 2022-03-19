#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "demi.h"

// https://freebsd.org/cgi/man.cgi?query=devctl&sektion=4
int demi_read(int fd, struct demi_event *de)
{
    struct msghdr hdr = {0};
    struct iovec iov = {0};

    char buf[8192], *msg_ptr, *pos;
    char *var_ptr, *key, *value;
    size_t value_len;
    ssize_t ret_len;

    if (!de) {
        errno = EINVAL;
        return -1;
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    ret_len = recvmsg(fd, &hdr, 0);

    if (ret_len <= 0) {
        return -1;
    }

    if (hdr.msg_flags & MSG_TRUNC) {
        return -1;
    }

    ret_len -= 1;
    assert(buf[ret_len] == '\n');
    buf[ret_len] = '\0';

    *de = (struct demi_event){0};
    pos = strtok_r(buf + 1, " ", &msg_ptr);

    if (!pos) {
        return 0;
    }

    do {
        key = strtok_r(pos, "=", &var_ptr);
        value = strtok_r(NULL, "=", &var_ptr);

        if (!key || !value) {
            continue;
        }

        if (strcmp(key, "cdev") == 0) {
            value_len = sizeof(de->de_devname);
            assert(strlcpy(de->de_devname, value, value_len) < value_len);
        }
        else if (strcmp(key, "type") != 0) {
            continue;
        }
        else if (strcmp(value, "CREATE") == 0) {
            de->de_type = DEMI_ATTACH;
        }
        else if (strcmp(value, "DESTROY") == 0) {
            de->de_type = DEMI_DETACH;
        }
        else if (strcmp(value, "HOTPLUG") == 0) {
            de->de_type = DEMI_CHANGE;
        }
        else {
            de->de_type = DEMI_UNKNOWN;
        }
    }
    while ((pos = strtok_r(NULL, " ", &msg_ptr)));

    return 0;
}

int demi_init(int flags)
{
    struct sockaddr_un sa = {0};
    int fd;

    fd = socket(AF_UNIX, SOCK_SEQPACKET | flags, 0);

    if (fd == -1) {
        return -1;
    }

    sa.sun_family = AF_UNIX;
    strlcpy(sa.sun_path, DEMI_MONITOR_DEVD_SOCKET, sizeof(sa.sun_path));

    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}
