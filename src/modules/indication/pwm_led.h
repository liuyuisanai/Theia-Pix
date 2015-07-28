#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PWM_LED_BLUE,
    PWM_LED_RED,
    PWM_LED_SIZE
};

void pwm_led_init();

// set led intensity 0 - 100
int pwm_led_set_intensity(int led, int intensity);

// led is active
int pwm_led_start(int led);

// led disabled
int pwm_led_stop(int led);

#ifdef __cplusplus
}
#endif
