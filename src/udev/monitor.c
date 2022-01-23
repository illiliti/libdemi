#include <libudev.h>

#include "demi.h"
#include "device.h"

int demi_monitor_recv_device(struct demi_monitor *dm, struct demi_device *dd)
{
    struct udev_device *udev_device;

    if (!dm || !dd) {
        return -1;
    }

    udev_device = udev_monitor_receive_device(dm->udev_monitor);

    if (!udev_device) {
        return -1;
    }

    if (device_init(dd, dm->ctx, udev_device) == -1) {
        udev_device_unref(udev_device);
        return -1;
    }

    return 0;
}

int demi_monitor_init(struct demi_monitor *dm, struct demi *ctx)
{
    if (!dm || !ctx) {
        return -1;
    }

    dm->udev_monitor = udev_monitor_new_from_netlink(ctx->udev, "udev");

    if (!dm->udev_monitor) {
        return -1;
    }

    if (udev_monitor_enable_receiving(dm->udev_monitor) < 0) {
        udev_monitor_unref(dm->udev_monitor);
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

    udev_monitor_unref(dm->udev_monitor);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? udev_monitor_get_fd(dm->udev_monitor) : -1;
}
