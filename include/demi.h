#include <sys/types.h>

#ifndef _DEMI_H_
#define _DEMI_H_

struct demi;
struct demi_device;
struct demi_monitor;
struct demi_enumerate;

// TODO extend
enum demi_device_type {
    DEMI_TYPE_UNKNOWN = 1,
    DEMI_TYPE_MOUSE,
    DEMI_TYPE_TABLET,
    DEMI_TYPE_TOUCHPAD,
    DEMI_TYPE_KEYBOARD,
    DEMI_TYPE_JOYSTICK,
    DEMI_TYPE_TOUCHSCREEN,
    DEMI_TYPE_SWITCH,
    DEMI_TYPE_ACCELEROMETER,
    DEMI_TYPE_POINTING_STICK,
    DEMI_TYPE_BOOT_VGA,
};

// TODO extend
enum demi_device_class {
    DEMI_CLASS_UNKNOWN = 1,
    DEMI_CLASS_DRM,
    DEMI_CLASS_INPUT,
};

enum demi_device_action {
    DEMI_ACTION_UNKNOWN = 1,
    DEMI_ACTION_ATTACH,
    DEMI_ACTION_DETACH,
    DEMI_ACTION_CHANGE,
    /* DEMI_ACTION_HOTPLUG, */
    /* DEMI_ACTION_LEASE, */
};

struct demi *demi_init(void);
void demi_deinit(struct demi *);

const char *demi_device_get_devnode(struct demi_device *);
const char *demi_device_get_devname(struct demi_device *);
dev_t demi_device_get_devnum(struct demi_device *);
int demi_device_get_devunit(struct demi_device *);

enum demi_device_action demi_device_get_action(struct demi_device *);
enum demi_device_class demi_device_get_class(struct demi_device *);
enum demi_device_type demi_device_get_type(struct demi_device *);

struct demi_device *demi_device_init_devnode(struct demi *, const char *);
struct demi_device *demi_device_init_devnum(struct demi *, dev_t, mode_t);
void demi_device_deinit(struct demi_device *);

int demi_monitor_get_fd(struct demi_monitor *);
struct demi_device *demi_monitor_recv_device(struct demi_monitor *);

struct demi_monitor *demi_monitor_init(struct demi *);
void demi_monitor_deinit(struct demi_monitor *);

int demi_enumerate_scan_system(struct demi_enumerate *,
        int (*)(struct demi_device *, void *), void *);

struct demi_enumerate *demi_enumerate_init(struct demi *);
void demi_enumerate_deinit(struct demi_enumerate *);

#endif
