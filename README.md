# libdemi

**!!! EXPERIMENTAL !!!**

Device enumeration, monitoring and introspecting library

## Showcase

```c
#include <demi.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int fd = demi_init(0);

    if (fd == -1) {
        return EXIT_FAILURE;
    }

    struct demi_event de;

    while (demi_read(fd, &de) != -1) {
        printf("%s (code %d)\n", de.de_devname, de.de_type);
    }

    close(fd);
    return EXIT_SUCCESS;
}
```

## See also

https://github.com/illiliti/kiss-libdemi
