#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/drvctlio.h>
#include <prop/proplib.h>

#include "demi.h"
#include "netbsd.h"

int demi_monitor_recv_devices(struct demi_monitor *dm,
        int (*cb)(struct demi_device *, void *), void *ptr)
{
    const char *action, *device;
	prop_dictionary_t event;
	struct demi_device *dd;

    if (!dm || !cb) {
        return -1;
    }

    while (prop_dictionary_recv_ioctl(dm->ctx->fd, DRVGETEVENT, &event) == 0) {
        prop_dictionary_get_cstring_nocopy(event, "event", &action);
        prop_dictionary_get_cstring_nocopy(event, "device", &device);

        if (strcmp(action, "device-attach") == 0) {
            dd = device_init(dm->ctx, NULL, device, 0, 0, DEMI_ACTION_ATTACH);
        }
        else if (strcmp(action, "device-detach") == 0) {
            dd = device_init(dm->ctx, NULL, device, 0, 0, DEMI_ACTION_DETACH);
        }
        else {
            abort();
        }

        prop_object_release(event);

        if (!dd) {
            return -1;
        }

        if (cb(dd, ptr) == -1) {
            return -1;
        }
	}

	return errno == EWOULDBLOCK ? 0 : -1;
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

    dm->ctx = ctx;
    return dm;
}

void demi_monitor_deinit(struct demi_monitor *dm)
{
    free(dm);
}

int demi_monitor_get_fd(struct demi_monitor *dm)
{
    return dm ? dm->ctx->fd : -1;
}
