#include <stdlib.h>
#include <string.h>
#include <sys/drvctlio.h>
#include <prop/proplib.h>

#include "demi.h"
#include "netbsd.h"

struct demi_device *demi_monitor_recv_device(struct demi_monitor *dm)
{
    const char *action, *device;
    prop_dictionary_t event;
    struct demi_device *dd;

    if (!dm) {
        return NULL;
    }

    if (prop_dictionary_recv_ioctl(dm->ctx->fd, DRVGETEVENT, &event) != 0) {
        return NULL;
    }

    prop_dictionary_get_cstring_nocopy(event, "event", &action);
    prop_dictionary_get_cstring_nocopy(event, "device", &device);

    if (strcmp(action, "device-attach") == 0) {
        dd = device_new(dm->ctx, NULL, device, 0, 0, DEMI_ACTION_ATTACH);
    }
    else if (strcmp(action, "device-detach") == 0) {
        dd = device_new(dm->ctx, NULL, device, 0, 0, DEMI_ACTION_DETACH);
    }
    else {
        abort();
    }

    prop_object_release(event);
    return dd;
}

struct demi_monitor *demi_monitor_new(struct demi *ctx)
{
    struct demi_monitor *dm;

    if (!ctx) {
        return NULL;
    }

    dm = malloc(sizeof(*dm));

    if (!dm) {
        return NULL;
    }

    dm->ctx = ctx;
    dm->ref = 1;
    return dm;
}

struct demi_monitor *demi_monitor_ref(struct demi_monitor *dm)
{
    if (!dm) {
        return NULL;
    }

    dm->ref++;
    return dm;
}

void demi_monitor_unref(struct demi_monitor *dm)
{
    if (!dm) {
        return;
    }

    if (--dm->ref > 0) {
        return;
    }

    free(dm);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? dm->ctx->fd : -1;
}
