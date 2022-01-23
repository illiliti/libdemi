#include <libudev.h>

#include "demi.h"
#include "device.h"

int demi_enumerate_scan_system(struct demi_enumerate *de,
        demi_enumerate_cb cb, void *ptr)
{
    struct udev_device *udev_device;
    struct udev_list_entry *entry;
    struct demi_device dd;

    if (udev_enumerate_scan_devices(de->udev_enumerate) < 0) {
        return -1;
    }

    entry = udev_enumerate_get_list_entry(de->udev_enumerate);

    do {
        udev_device = udev_device_new_from_syspath(de->ctx->udev,
                udev_list_entry_get_name(entry));

        if (!udev_device) {
            return -1;
        }

        if (device_init(&dd, de->ctx, udev_device) == -1) {
            udev_device_unref(udev_device);
            return -1;
        }

        if (cb(&dd, ptr) == -1) {
            return -1;
        }
    }
    while ((entry = udev_list_entry_get_next(entry)));

    return 0;
}

int demi_enumerate_init(struct demi_enumerate *de, struct demi *ctx)
{
    if (!de || !ctx) {
        return -1;
    }

    de->udev_enumerate = udev_enumerate_new(ctx->udev);

    if (!de->udev_enumerate) {
        return -1;
    }

    de->ctx = ctx;
    return 0;
}

void demi_enumerate_finish(struct demi_enumerate *de)
{
    if (!de) {
        return;
    }

    udev_enumerate_unref(de->udev_enumerate);
}
