#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "demi.h"
#include "linux.h"

struct demi_device *demi_monitor_recv_device(struct demi_monitor *dm)
{
    struct sockaddr_nl sa = {0};
    struct msghdr hdr = {0};
    struct iovec iov = {0};
    char buf[8192];
    ssize_t len;

    if (!dm) {
        return NULL;
    }

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);

    hdr.msg_name = &sa;
    hdr.msg_namelen = sizeof(sa);
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;

    len = recvmsg(dm->fd, &hdr, 0);

    if (len <= 0) {
        return NULL;
    }

    if (hdr.msg_flags & MSG_TRUNC) {
        return NULL;
    }

    if (sa.nl_groups == 0x0 || (sa.nl_groups == 0x1 && sa.nl_pid != 0)) {
        return NULL;
    }

    return device_init_uevent(dm->ctx, buf, len);
}

struct demi_monitor *demi_monitor_init(struct demi *ctx)
{
    struct sockaddr_nl sa = {0};
    struct demi_monitor *dm;

    if (!ctx) {
        return NULL;
    }

    dm = malloc(sizeof(*dm));

    if (!dm) {
        return NULL;
    }

    dm->fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
            NETLINK_KOBJECT_UEVENT);

    if (dm->fd == -1) {
        free(dm);
        return NULL;
    }

    sa.nl_family = AF_NETLINK;
    sa.nl_groups = DEMI_MONITOR_NETLINK_GROUP;

    if (bind(dm->fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        close(dm->fd);
        free(dm);
        return NULL;
    }

    dm->ctx = ctx;
    return dm;
}

void demi_monitor_deinit(struct demi_monitor *dm)
{
    if (!dm) {
        return;
    }

    close(dm->fd);
    free(dm);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? dm->fd : -1;
}
