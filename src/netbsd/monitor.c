#include <stdlib.h>
#include <string.h>
#include <sys/drvctlio.h>
#include <prop/proplib.h>

#include "demi.h"
#include "device.h"

int demi_monitor_recv_device(struct demi_monitor *dm, struct demi_device *dd)
{
    const char *action, *device;
    prop_dictionary_t event;
    int ret;

    if (!dm || !dd) {
        return -1;
    }

    if (prop_dictionary_recv_ioctl(dm->ctx->fd, DRVGETEVENT, &event) != 0) {
        return -1;
    }

    prop_dictionary_get_cstring_nocopy(event, "event", &action);
    prop_dictionary_get_cstring_nocopy(event, "device", &device);

    if (strcmp(action, "device-attach") == 0) {
        ret = device_init(dd, dm->ctx, NULL, device, 0, 0, DEMI_ACTION_ATTACH);
    }
    else if (strcmp(action, "device-detach") == 0) {
        ret = device_init(dd, dm->ctx, NULL, device, 0, 0, DEMI_ACTION_DETACH);
    }
    else {
        abort();
    }

    prop_object_release(event);
    return ret;
}

int demi_monitor_init(struct demi_monitor *dm, struct demi *ctx)
{
    if (!dm || !ctx) {
        return -1;
    }

    dm->ctx = ctx;
    return 0;
}

void demi_monitor_finish(struct demi_monitor *dm)
{
    (void)dm;
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? dm->ctx->fd : -1;
}
