# libdemi

**!!! EXPERIMENTAL !!!**

Device enumeration, monitoring and introspecting library

## Showcase

```c
#include <demi.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

static int callback(struct demi_device *dev, void *ptr)
{
    // You can save dev for later use(outside of callback) if needed.
    //
    // struct your_struct *data = ptr;
    // data->dev = *dev;

    // Do something with dev...

    demi_device_finish(dev);
    return 0;
}

int main(void)
{
    int ret = EXIT_SUCCESS;

    struct demi ctx;

    if (demi_init(&ctx) == -1) {
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    struct demi_enumerate enu;

    if (demi_enumerate_init(&enu, &ctx) == -1) {
        ret = EXIT_FAILURE;
        goto finish_ctx;
    }

    // Scan connected devices. callback will be called on each device.
    if (demi_enumerate_scan_system(&enu, callback, NULL) == -1) {
        ret = EXIT_FAILURE;
        goto finish_enu;
    }

    struct demi_monitor mon;

    if (demi_monitor_init(&mon, &ctx) == -1) {
        ret = EXIT_FAILURE;
        goto finish_enu;
    }

    struct poll pfd;

    pfd.events = POLLIN;
    pfd.fd = demi_monitor_get_fd(&mon);

    if (poll(&pfd, 1, -1) == -1) {
        ret = EXIT_FAILURE;
        goto finish_mon;
    }

    struct demi_device dev;

    // This function may block if blocking mode was manually set to fd above.
    if (demi_monitor_recv_device(&mon, &dev) == -1) {
        ret = EXIT_FAILURE;
        goto finish_mon;
    }

    // Do something with dev...

    demi_device_finish(&dev);
finish_mon:
    demi_monitor_finish(&mon);
finish_enu:
    demi_enumerate_finish(&enu);
finish_ctx:
    demi_finish(&ctx);
do_exit:
    return ret;
}
```
