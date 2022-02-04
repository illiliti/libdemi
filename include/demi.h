#include <stdint.h>
#include <sys/types.h>

#ifndef _DEMI_H_
#define _DEMI_H_

#include "demi_enums.h"
#include "demi_structs.h"

int demi_init(struct demi *context);
void demi_finish(struct demi *context);

int demi_device_get_devnode(struct demi_device *device, const char **devnode);
int demi_device_get_devname(struct demi_device *device, const char **devname);
int demi_device_get_devunit(struct demi_device *device, uint32_t *devunit);
int demi_device_get_devnum(struct demi_device *device, dev_t *devnum);

int demi_device_get_action(struct demi_device *device,
        enum demi_action *action);
int demi_device_get_class(struct demi_device *device, enum demi_class *class);
int demi_device_get_type(struct demi_device *device, uint32_t *type);
int demi_device_get_seat(struct demi_device *device, const char **seat);

int demi_device_init_devnode(struct demi_device *device, struct demi *context,
        const char *devnode);
int demi_device_init_devnum(struct demi_device *device, struct demi *context,
        dev_t devnum, mode_t type);
void demi_device_finish(struct demi_device *device);

int demi_monitor_get_fd(struct demi_monitor *monitor);
int demi_monitor_recv_device(struct demi_monitor *monitor,
        struct demi_device *device);

int demi_monitor_init(struct demi_monitor *monitor, struct demi *context);
void demi_monitor_finish(struct demi_monitor *monitor);

typedef int (*demi_enumerate_cb)(struct demi_device *device, void *userdata);
int demi_enumerate_scan_system(struct demi_enumerate *enumerate,
        demi_enumerate_cb callback, void *userdata);

int demi_enumerate_init(struct demi_enumerate *enumerate, struct demi *context);
void demi_enumerate_finish(struct demi_enumerate *enumerate);

#endif
