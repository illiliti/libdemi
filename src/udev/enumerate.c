#include <stdlib.h>
#include <libudev.h>

#include "demi.h"
#include "udev.h"

int demi_enumerate_scan_system(struct demi_enumerate *de,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    struct udev_device *udev_device;
    struct udev_list_entry *list;
    struct demi_device *dd;

    if (udev_enumerate_scan_devices(de->udev_enumerate) < 0) {
        return -1;
    }

    list = udev_enumerate_get_list_entry(de->udev_enumerate);

    do {
        udev_device = udev_device_new_from_syspath(de->ctx->udev,
                udev_list_entry_get_name(list));

        if (!udev_device) {
            return -1;
        }

        dd = device_new(de->ctx, udev_device);

        if (!dd) {
            udev_device_unref(udev_device);
            return -1;
        }

        if (cb(dd, ptr) == -1) {
            return -1;
        }
    }
    while ((list = udev_list_entry_get_next(list)));

    return 0;
}

struct demi_enumerate *demi_enumerate_new(struct demi *ctx)
{
    struct demi_enumerate *de;

    if (!ctx) {
        return NULL;
    }

    de = malloc(sizeof(*de));

    if (!de) {
        return NULL;
    }

    de->udev_enumerate = udev_enumerate_new(ctx->udev);

    if (!de->udev_enumerate) {
        free(de);
        return NULL;
    }

    de->ctx = ctx;
    de->ref = 1;
    return de;
}

struct demi_enumerate *demi_enumerate_ref(struct demi_enumerate *de)
{
    if (!de) {
        return NULL;
    }

    de->ref++;
    return de;
}

void demi_enumerate_unref(struct demi_enumerate *de)
{
    if (!de) {
        return;
    }

    if (--de->ref > 0) {
        return;
    }

    udev_enumerate_unref(de->udev_enumerate);
    free(de);
}
