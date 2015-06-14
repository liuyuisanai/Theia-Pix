#include "i2c_controller.h"

#include <nuttx/config.h>
#include <nuttx/clock.h>

#include <drivers/drv_hrt.h>

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include <systemlib/perf_counter.h>
#include <systemlib/err.h>
#include <systemlib/systemlib.h>

#define CONTROLLER_PATH "/dev/buttons"
#define CONTROLLER_NAME "buttons"
#define CONTROLLER_SETUP_COMMAND 0x00
#define LED_SETUP_PORT_NUMBER 0x07
#define LED_SETUP_CMD 0x3F
#define LED_WRITE_PORT_NUMBER 0x03
#define MSECS_IN_SEC 1000

I2C_CONTROLLER::I2C_CONTROLLER(int bus, int addr) :
	I2C(CONTROLLER_NAME, CONTROLLER_PATH, bus, addr, DEVICE_FREQUENCY),
    _listening_interval(USEC2TICK(DEVICE_FREQUENCY))
{
    memset(&_green_led, 0, sizeof(_green_led));
    memset(&_red_led, 0, sizeof(_red_led));
}

int
I2C_CONTROLLER::init(cButtonController *pbutton_ctrl)
{
	int ret;
	ret = I2C::init();

	if (ret != OK) {
		return ret;
	}

	m_pbutton_ctrl = pbutton_ctrl;

    ret = init_led();
    if (ret != OK) {
        return ret;
    }

    _led_update_in_progress = false;

    _green_led = {I2C_LED_GREEN, BLINKING_RATE_SLOW, 0, false, false};
    _red_led = {I2C_LED_RED, BLINKING_RATE_SLOW, 0, false, false};

	return OK;
}

int
I2C_CONTROLLER::probe()
{
    uint8_t response[1] = {0};
    uint8_t requests[1] = {CONTROLLER_SETUP_COMMAND};

    int ret = transfer(&requests[0], sizeof(requests), nullptr, 0);
    ret = transfer(nullptr, 0, response, sizeof(response));
	return ret;
}

int
I2C_CONTROLLER::init_led()
{
    uint8_t requests[2] = {LED_SETUP_PORT_NUMBER, LED_SETUP_CMD};
    int ret = transfer(requests, sizeof(requests), nullptr, 0);
    return ret;
}

void
I2C_CONTROLLER::cycle()
{
    if (!_led_update_in_progress) {
		uint32_t button_mask = 0;

		button_mask |= get_buttons_state_from_r1() << 0;
		button_mask |= get_buttons_state_from_r2() << 8;

		m_pbutton_ctrl->setMask(BS_I2C, button_mask);
    }

    uint64_t now = hrt_absolute_time();
    if (_green_led.should_blink) {
        if ((now - _green_led.last_blink) / MSECS_IN_SEC < _green_led.rate) {
            set_green_led_on(false);
        } else {
            set_green_led_on(true);
            _green_led.last_blink = now;
        }
    }

    if (_red_led.should_blink) {
        if ((now - _red_led.last_blink) / MSECS_IN_SEC < _red_led.rate) {
            set_red_led_on(false);
        } else {
            set_red_led_on(true);
            _red_led.last_blink = now;
        }
    }
}

int
I2C_CONTROLLER::get_buttons_state_from_r1()
{
    uint8_t response[1] = {0};
    // I do not know why it is 0x00, but it works
    uint8_t requests[1] = {0x00};

    int ret = transfer(&requests[0], sizeof(requests), nullptr, 0);
    ret = transfer(nullptr, 0, response, sizeof(response));

    if (ret == OK) {
        return response[0];
    }
	return -1;
}

int
I2C_CONTROLLER::get_buttons_state_from_r2()
{
    uint8_t response[1] = {0};
    // This is according to specs. Fishy.
    uint8_t requests[2] = {0x00, 0x41};

    int ret = transfer(requests, sizeof(requests), nullptr, 0);
    ret = transfer(nullptr, 0, response, sizeof(response));

    if (ret == OK) {
        return response[0];
    }
	return -1;
}

void
I2C_CONTROLLER::set_green_led_on(bool set_on)
{
    uint8_t green = set_on ? 0x40 : 0x00;
    uint8_t red = _is_red_led_on ? 0x80 : 0x00;
    uint8_t state = 0xFF ^ green ^ red;

    uint8_t requests[2] = {LED_WRITE_PORT_NUMBER, state};
    int ret = transfer(requests, sizeof(requests), nullptr, 0);
    if (ret == OK) {
       _is_green_led_on = set_on;
    }
}

void
I2C_CONTROLLER::set_red_led_on(bool set_on)
{
    uint8_t green = _is_green_led_on ? 0x40 : 0x00;
    uint8_t red = set_on ? 0x80 : 0x00;
    uint8_t state = 0xFF ^ green ^ red;

    uint8_t requests[2] = {LED_WRITE_PORT_NUMBER, state};
    int ret = transfer(requests, sizeof(requests), nullptr, 0);
    if (ret == OK) {
       _is_red_led_on = set_on;
    }
}

/*
 * BLinking API
 */

void
I2C_CONTROLLER::start_blinking_led(i2c_led_t led, blinking_rate_t rate)
{
    uint64_t now = hrt_absolute_time();
    switch (led) {
        case I2C_LED_GREEN:
            if (_green_led.should_blink) {
                return;
            }
            _green_led.rate = rate;
            _green_led.should_blink = true;
            _green_led.was_on = _is_green_led_on;
            _green_led.last_blink = _red_led.last_blink == 0 ? now : _red_led.last_blink;
            break;
        case I2C_LED_RED:
            if (_red_led.should_blink) {
                return;
            }
            _red_led.rate = rate;
            _red_led.should_blink = true;
            _red_led.was_on = _is_red_led_on;
            _red_led.last_blink = _green_led.last_blink == 0 ? now : _green_led.last_blink;
            break;
        default:
            break;
    }
}

void
I2C_CONTROLLER::stop_blinking_led(i2c_led_t led, bool should_restore_state)
{
    switch (led) {
        case I2C_LED_GREEN:
            _green_led.should_blink = false;
            _green_led.last_blink = 0;
            set_red_led_on(_green_led.was_on);
            break;
        case I2C_LED_RED:
            _red_led.should_blink = false;
            _red_led.last_blink = 0;
            set_green_led_on(_red_led.was_on);
            break;
        default:
            break;
    }
}
