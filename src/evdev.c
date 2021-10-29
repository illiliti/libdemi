#include "demi.h"
#include "evdev.h"

static inline int test_bit(unsigned long *arr, unsigned long bit)
{
    return !!(arr[BIT_WORD(bit)] & BIT_MASK(bit));
}

enum demi_type parse_evdev(struct evdev *evdev)
{
    if (test_bit(evdev->prop, INPUT_PROP_POINTING_STICK)) {
        return DEMI_TYPE_POINTING_STICK;
    }

    if (test_bit(evdev->prop, INPUT_PROP_ACCELEROMETER)) {
        return DEMI_TYPE_ACCELEROMETER;
    }

    if (test_bit(evdev->ev, EV_SW)) {
        return DEMI_TYPE_SWITCH;
    }

    if (test_bit(evdev->ev, EV_REL)) {
        if (test_bit(evdev->rel, REL_Y) && test_bit(evdev->rel, REL_X) &&
            test_bit(evdev->key, BTN_MOUSE)) {
            return DEMI_TYPE_MOUSE;
        }
    }
    else if (test_bit(evdev->ev, EV_ABS)) {
        if (test_bit(evdev->key, BTN_SELECT) || test_bit(evdev->key, BTN_TR) ||
            test_bit(evdev->key, BTN_START) || test_bit(evdev->key, BTN_TL)) {
            if (test_bit(evdev->key, BTN_TOUCH)) {
                return DEMI_TYPE_TOUCHSCREEN;
            }
            else {
                return DEMI_TYPE_JOYSTICK;
            }
        }
        else if (test_bit(evdev->abs, ABS_Y) && test_bit(evdev->abs, ABS_X)) {
            if (test_bit(evdev->abs, ABS_Z) && !test_bit(evdev->ev, EV_KEY)) {
                return DEMI_TYPE_ACCELEROMETER;
            }
            else if (test_bit(evdev->key, BTN_STYLUS) || test_bit(evdev->key, BTN_TOOL_PEN)) {
                return DEMI_TYPE_TABLET;
            }
            else if (test_bit(evdev->key, BTN_TOUCH)) {
                if (test_bit(evdev->key, BTN_TOOL_FINGER)) {
                    return DEMI_TYPE_TOUCHPAD;
                }
                else {
                    return DEMI_TYPE_TOUCHSCREEN;
                }
            }
            else if (test_bit(evdev->key, BTN_MOUSE)) {
                return DEMI_TYPE_MOUSE;
            }
        }
    }

    if (test_bit(evdev->key, KEY_ENTER)) {
        return DEMI_TYPE_KEYBOARD;
    }

    return DEMI_TYPE_UNKNOWN;
}
