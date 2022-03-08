# libdemi

**!!! EXPERIMENTAL !!!**

Device enumeration, monitoring and introspecting library

## Showcase

```c
#include <demi.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main(void)
{
    int ret = EXIT_SUCCESS;

    struct demi ctx;

    if (demi_init(&ctx) == -1) {
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    // Enumerate drm devices.
    DIR *dp = opendir("/dev/dri");

    if (!dp) {
        ret = EXIT_FAILURE;
        goto finish_ctx;
    }

    int dfd = dirfd(dp);
    struct dirent *de;

    while ((de = readdir(dp))) {
        struct stat st;

        if (fstatat(dfd, de->d_name, &st, 0) == -1) {
            continue;
        }

        mode_t type = st.st_mode & (S_IFCHR | S_IFBLK);

        if (!type) {
            continue;
        }

        struct demi_device dev;

        if (demi_device_init_devnum(&dev, &ctx, st.st_rdev, type) == -1) {
            continue;
        }

        const char *devnode;

        if (demi_device_get_devnode(&dev, &devnode) == 0) {
            puts(devnode);
        }

        demi_device_finish(&dev);
    }

    closedir(dp);

    struct demi_monitor mon;

    if (demi_monitor_init(&mon, &ctx) == -1) {
        ret = EXIT_FAILURE;
        goto finish_ctx;
    }

    int fd = demi_monitor_get_fd(&mon);
    int flags = fcntl(fd, F_GETFL);

    if (flags == -1) {
        ret = EXIT_FAILURE;
        goto finish_mon;
    }

    flags &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        ret = EXIT_FAILURE;
        goto finish_mon;
    }

    struct demi_device dev;

    // This function will block since fd was set to blocking mode.
    if (demi_monitor_recv_device(&mon, &dev) == -1) {
        ret = EXIT_FAILURE;
        goto finish_mon;
    }

    const char *devnode;

    if (demi_device_get_devnode(&dev, &devnode) == 0) {
        puts(devnode);
    }

    demi_device_finish(&dev);
finish_mon:
    demi_monitor_finish(&mon);
finish_ctx:
    demi_finish(&ctx);
do_exit:
    return ret;
}
```

## See also

https://github.com/illiliti/kiss-libdemi
