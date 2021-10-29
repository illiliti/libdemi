#include <limits.h>

#if defined(__linux__)
#include <linux/input.h>
#elif defined(__FreeBSD__)
#include <dev/evdev/input.h>
#elif defined(__DragonFly__)
#include <dev/misc/evdev/input.h>
#endif

#include "demi.h"

#ifndef LONG_BIT
#define LONG_BIT (sizeof(unsigned long) * 8)
#endif

#define BIT_WORD(x) ((x) / LONG_BIT)
#define BIT_MASK(x) (1UL << ((x) % LONG_BIT))
#define BITS_TO_LONGS(x) (((x) + LONG_BIT - 1) / LONG_BIT)

#ifndef INPUT_PROP_POINTING_STICK
#define INPUT_PROP_POINTING_STICK 0x05
#endif

#ifndef INPUT_PROP_ACCELEROMETER
#define INPUT_PROP_ACCELEROMETER 0x06
#endif

#ifndef INPUT_PROP_CNT
#define INPUT_PROP_CNT 0x20
#endif

struct evdev {
    unsigned long prop[BITS_TO_LONGS(INPUT_PROP_CNT)];
    unsigned long abs[BITS_TO_LONGS(ABS_CNT)];
    unsigned long rel[BITS_TO_LONGS(REL_CNT)];
    unsigned long key[BITS_TO_LONGS(KEY_CNT)];
    unsigned long ev[BITS_TO_LONGS(EV_CNT)];
};

enum demi_type parse_evdev(struct evdev *);
