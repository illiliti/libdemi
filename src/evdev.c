#include <stdint.h>

#include "demi.h"
#include "evdev.h"

static inline int test_bit(unsigned long *arr, unsigned long bit)
{
    return !!(arr[BIT_WORD(bit)] & BIT_MASK(bit));
}

uint32_t parse_evdev(struct evdev *evdev)
{
    uint32_t type = 0;
    unsigned long bit;

    if (test_bit(evdev->prop, INPUT_PROP_POINTING_STICK)) {
        type |= DEMI_TYPE_POINTING_STICK;
    }

    if (test_bit(evdev->prop, INPUT_PROP_ACCELEROMETER)) {
        type |= DEMI_TYPE_ACCELEROMETER;
    }

    if (test_bit(evdev->ev, EV_SW)) {
        type |= DEMI_TYPE_SWITCH;
    }

    if (test_bit(evdev->ev, EV_REL)) {
        if (test_bit(evdev->rel, REL_Y) && test_bit(evdev->rel, REL_X) &&
            test_bit(evdev->key, BTN_MOUSE)) {
            type |= DEMI_TYPE_MOUSE;
        }
    }
    else if (test_bit(evdev->ev, EV_ABS)) {
        if (test_bit(evdev->key, BTN_SELECT) || test_bit(evdev->key, BTN_TR) ||
            test_bit(evdev->key, BTN_START) || test_bit(evdev->key, BTN_TL)) {
            if (test_bit(evdev->key, BTN_TOUCH)) {
                type |= DEMI_TYPE_TOUCHSCREEN;
            }
            else {
                type |= DEMI_TYPE_JOYSTICK;
            }
        }
        else if (test_bit(evdev->abs, ABS_Y) && test_bit(evdev->abs, ABS_X)) {
            if (test_bit(evdev->abs, ABS_Z) && !test_bit(evdev->ev, EV_KEY)) {
                type |= DEMI_TYPE_ACCELEROMETER;
            }
            else if (test_bit(evdev->key, BTN_STYLUS) || test_bit(evdev->key,
                         BTN_TOOL_PEN)) {
                type |= DEMI_TYPE_TABLET;
            }
            else if (test_bit(evdev->key, BTN_TOUCH)) {
                if (test_bit(evdev->key, BTN_TOOL_FINGER)) {
                    type |= DEMI_TYPE_TOUCHPAD;
                }
                else {
                    type |= DEMI_TYPE_TOUCHSCREEN;
                }
            }
            else if (test_bit(evdev->key, BTN_MOUSE)) {
                type |= DEMI_TYPE_MOUSE;
            }
        }
    }

    for (bit = KEY_ESC; bit < BTN_MISC; bit++) {
        if (test_bit(evdev->key, bit)) {
            type |= DEMI_TYPE_KEY;

            if (test_bit(evdev->key, KEY_ENTER)) {
                type |= DEMI_TYPE_KEYBOARD;
            }

            break;
        }
    }

    return type ? type : DEMI_TYPE_UNKNOWN;
}
