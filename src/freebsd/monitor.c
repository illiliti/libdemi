#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "demi.h"
#include "device.h"

// https://freebsd.org/cgi/man.cgi?query=devctl&sektion=4
int demi_monitor_recv_device(struct demi_monitor *dm, struct demi_device *dd)
{
    struct msghdr hdr = {0};
    struct iovec iov = {0};
    char *pos, *msg;
    char buf[8192];
    ssize_t len;

    if (!dm || !dd) {
        return -1;
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    len = recvmsg(dm->fd, &hdr, 0);

    if (len <= 0) {
        return -1;
    }

    if (hdr.msg_flags & MSG_TRUNC) {
        return -1;
    }

    len -= 1;
    assert(buf[len] == '\n');
    buf[len] = '\0';
    msg = buf + 1; // strip msg type

    pos = msg;
    while ((pos = strchr(pos, ' '))) {
        *pos = '\0'; // convert msg to uevent style
        pos += 1;
    }

    return device_init_msg(dd, dm->ctx, msg, (size_t)len);
}

int demi_monitor_init(struct demi_monitor *dm, struct demi *ctx)
{
    struct sockaddr_un sa = {0};

    if (!dm || !ctx) {
        return -1;
    }

    dm->fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);

    if (dm->fd == -1) {
        return -1;
    }

    sa.sun_family = AF_UNIX;
    strlcpy(sa.sun_path, DEMI_MONITOR_DEVD_SOCKET, sizeof(sa.sun_path));

    if (connect(dm->fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        close(dm->fd);
        return -1;
    }

    dm->ctx = ctx;
    return 0;
}

void demi_monitor_finish(struct demi_monitor *dm)
{
    if (!dm) {
        return;
    }

    close(dm->fd);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? dm->fd : -1;
}
