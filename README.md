# libdemi

**!!! EXPERIMENTAL !!!**

Device enumeration, monitoring and introspecting library

## Showcase

```c
#include <demi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    // Initialize demi file descriptor with no/zero flags.
    // Optionally, DEMI_CLOEXEC and DEMI_NONBLOCK can be bitwise ORed in flags
    // to atomically set close-on-exec flag and nonblocking mode respectively.
    int fd = demi_init(0);

    if (fd == -1) {
        return EXIT_FAILURE;
    }

    struct demi_event de;

    // Read pending event. This call will block since DEMI_NONBLOCK was not set.
    while (demi_read(fd, &de) != -1) {
        // de_devname might not contain devname, indicating that the event shall be ignored.
        if (de.de_devname[0] == '\0') {
            continue;
        }

        // Prepend /dev/ to devname, so that we have full path to devnode.
        char devnode[sizeof(de.de_devname) + sizeof("/dev/")];
        snprintf(devnode, sizeof(devnode), "/dev/%s", de.de_devname);

        // Print devnode and code of event type.
        printf("%s (code %d)\n", de.de_devname, de.de_type);
    }

    // Do not forget to close file descriptor when you are done.
    close(fd);
    return EXIT_SUCCESS;
}
```

## See also

https://github.com/illiliti/kiss-libdemi
