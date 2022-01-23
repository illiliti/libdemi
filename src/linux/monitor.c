#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "demi.h"
#include "device.h"

int demi_monitor_recv_device(struct demi_monitor *dm, struct demi_device *dd)
{
    struct sockaddr_nl sa = {0};
    struct msghdr hdr = {0};
    struct iovec iov = {0};
    char buf[8192];
    ssize_t len;

    if (!dm || !dd) {
        return -1;
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    hdr.msg_name = &sa;
    hdr.msg_namelen = sizeof(sa);
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    len = recvmsg(dm->fd, &hdr, 0);

    if (len <= 0) {
        return -1;
    }

    if (hdr.msg_flags & MSG_TRUNC) {
        return -1;
    }

    if (sa.nl_groups == 0x0 || (sa.nl_groups == 0x1 && sa.nl_pid != 0)) {
        return -1;
    }

    return device_init_uevent(dd, dm->ctx, buf, len);
}

int demi_monitor_init(struct demi_monitor *dm, struct demi *ctx)
{
    struct sockaddr_nl sa = {0};

    if (!dm || !ctx) {
        return -1;
    }

    dm->fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
            NETLINK_KOBJECT_UEVENT);

    if (dm->fd == -1) {
        return -1;
    }

    sa.nl_family = AF_NETLINK;
    sa.nl_groups = DEMI_MONITOR_NETLINK_GROUP;

    if (bind(dm->fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
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
