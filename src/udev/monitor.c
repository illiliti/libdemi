#include <stdlib.h>
#include <libudev.h>

#include "demi.h"
#include "udev.h"

struct demi_device *demi_monitor_recv_device(struct demi_monitor *dm)
{
    struct udev_device *udev_device;
    struct demi_device *dd;

    if (!dm) {
        return NULL;
    }

    udev_device = udev_monitor_receive_device(dm->udev_monitor);

    if (!udev_device) {
        return NULL;
    }

    dd = device_init(dm->ctx, udev_device);

    if (!dd) {
        udev_device_unref(udev_device);
        return NULL;
    }

    return dd;
}

struct demi_monitor *demi_monitor_init(struct demi *ctx)
{
    struct demi_monitor *dm;

    if (!ctx) {
        return NULL;
    }

    dm = malloc(sizeof(*dm));

    if (!dm) {
        return NULL;
    }

    dm->udev_monitor = udev_monitor_new_from_netlink(ctx->udev, "udev");

    if (!dm->udev_monitor) {
        free(dm);
        return NULL;
    }

    if (udev_monitor_enable_receiving(dm->udev_monitor) < 0) {
        udev_monitor_unref(dm->udev_monitor);
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

    udev_monitor_unref(dm->udev_monitor);
    free(dm);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? udev_monitor_get_fd(dm->udev_monitor) : -1;
}
