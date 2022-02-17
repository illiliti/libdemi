// TODO extend
enum demi_type {
    DEMI_TYPE_UNKNOWN = 1 << 0,
    DEMI_TYPE_MOUSE = 1 << 1,
    DEMI_TYPE_TABLET = 1 << 2,
    DEMI_TYPE_TOUCHPAD = 1 << 3,
    DEMI_TYPE_KEYBOARD = 1 << 4,
    DEMI_TYPE_JOYSTICK = 1 << 5,
    DEMI_TYPE_TOUCHSCREEN = 1 << 6,
    DEMI_TYPE_SWITCH = 1 << 7,
    DEMI_TYPE_ACCELEROMETER = 1 << 8,
    DEMI_TYPE_POINTING_STICK = 1 << 9,
    DEMI_TYPE_KEY = 1 << 10,

    DEMI_TYPE_BOOT_VGA = 1 << 11,
};

// TODO extend
enum demi_class {
    DEMI_CLASS_UNKNOWN = 1,
    DEMI_CLASS_DRM,
    DEMI_CLASS_INPUT,
};

enum demi_action {
    DEMI_ACTION_UNKNOWN = 1,
    DEMI_ACTION_ATTACH,
    DEMI_ACTION_DETACH,
    DEMI_ACTION_CHANGE,
    // DEMI_ACTION_HOTPLUG,
    // DEMI_ACTION_LEASE,
};
