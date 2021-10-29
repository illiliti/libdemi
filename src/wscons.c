#include <string.h>

#include "demi.h"
#include "wscons.h"

enum demi_type parse_wscons(const char *devname)
{
    // TODO lookup table
    if (strncmp(devname, "wskbd", 5) == 0) {
        return DEMI_TYPE_KEYBOARD;
    }
    else if (strncmp(devname, "wsmouse", 7) == 0) {
        return DEMI_TYPE_MOUSE;
    }
    else if (strncmp(devname, "joy", 3) == 0) {
        return DEMI_TYPE_JOYSTICK;
    }

    return DEMI_TYPE_UNKNOWN;
}
