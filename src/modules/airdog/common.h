/*
 *
 * Shared constants and enums
 *
 */

#ifndef HAVE_ENUM_MAV_PARAM_TYPE
#define HAVE_ENUM_MAV_PARAM_TYPE

#include <unistd.h>

#define DEVICE_FREQUENCY 100000

enum MAV_PARAM_TYPE
{
	MAV_PARAM_TYPE_UINT8=1, /* 8-bit unsigned integer | */
	MAV_PARAM_TYPE_INT8=2, /* 8-bit signed integer | */
	MAV_PARAM_TYPE_UINT16=3, /* 16-bit unsigned integer | */
	MAV_PARAM_TYPE_INT16=4, /* 16-bit signed integer | */
	MAV_PARAM_TYPE_UINT32=5, /* 32-bit unsigned integer | */
	MAV_PARAM_TYPE_INT32=6, /* 32-bit signed integer | */
	MAV_PARAM_TYPE_UINT64=7, /* 64-bit unsigned integer | */
	MAV_PARAM_TYPE_INT64=8, /* 64-bit signed integer | */
	MAV_PARAM_TYPE_REAL32=9, /* 32-bit floating-point | */
	MAV_PARAM_TYPE_REAL64=10, /* 64-bit floating-point | */
	MAV_PARAM_TYPE_ENUM_END=11, /*  | */
};

typedef enum {
    BLINKING_RATE_SLOW = 1000, // 1 Hz
    BLINKING_RATE_MEDIUM = 500, // 2 Hz
    BLINKING_RATE_FAST = 250, // 4 Hz
} blinking_rate_t;

typedef enum {
    I2C_LED_GREEN = 0,
    I2C_LED_RED,
} i2c_led_t;

struct led_s {
    i2c_led_t led;
    blinking_rate_t rate;
    uint64_t last_blink;
    bool should_blink;
    bool was_on;
};

#define BUTTON_COUNT_I2C 9
#define BUTTON_COUNT_GPIO 6

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#endif
