#include <nuttx/config.h>
#include <nuttx/fs/fs.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <stm32.h>

#include <board_config.h>
#include <drivers/drv_airleash_kbd.h>
#include <drivers/drv_hrt.h>

#include "bounce_filter.hpp"
#include "buffer.hpp"
#include "config_helper.hpp"
#include "driver.h"
#include "state.hpp"
#include "stm32_helper.hpp"

#define message(...)	printf(__VA_ARGS__)

namespace airleash_kbd {

// Type checks

static_assert(count_args(GPIO_KBD_INPUT_PINS) == GPIO_KBD_N_INPUTS,
		"GPIO_KBD INPUT lines configuration mismatch.");

static_assert((pressed_mask_t(1) << GPIO_KBD_N_KEYS) != 0,
		"pressed_mask_t too small");

// Buffer definition
using buffer_size_t = uint8_t;
static_assert(buffer_size_t(KBD_SCAN_BUFFER_N_ITEMS) * 2 != 0,
		"buffer_size_t is too small.");
using buffer_t = AlwaysFullBuffer<
	pressed_mask_t,
	buffer_size_t,
	KBD_SCAN_BUFFER_N_ITEMS * 2,
	volatile buffer_size_t
>;

// Globals
static buffer_t buffer;

namespace scan_gpios {

static struct hrt_call               hrt_scan_entry;
static unsigned                      output_n;
static ScanState                     state;
static BounceFilter<pressed_mask_t>  bounce_filter;

static void
switch_output_line()
{
	constexpr bool ON = GPIO_KBD_OUT_ACTIVE_HIGH;
	constexpr bool off = !ON;

	switch (output_n)
	{
	case 0:
		output_n = 1;
		stm32_gpiowrite(GPIO_KBD_OUT_0, off);
		stm32_gpiowrite(GPIO_KBD_OUT_1, ON);
		stm32_gpiowrite(GPIO_KBD_OUT_2, off);
		break;
	case 1:
		output_n = 2;
		stm32_gpiowrite(GPIO_KBD_OUT_0, off);
		stm32_gpiowrite(GPIO_KBD_OUT_1, off);
		stm32_gpiowrite(GPIO_KBD_OUT_2, ON);
		break;
	case 2:
		output_n = 0;
		stm32_gpiowrite(GPIO_KBD_OUT_0, ON);
		stm32_gpiowrite(GPIO_KBD_OUT_1, off);
		stm32_gpiowrite(GPIO_KBD_OUT_2, off);
		break;
	}
}

static reg_value_t
read_input_port()
{
	reg_value_t port = stm32_gpioread_port(GPIO_KBD_INPUT_PORT);
#if not GPIO_KBD_IN_ACTIVE_HIGH
	port = ~port;
#endif
	return port & gpio_reg_pin_mask<GPIO_KBD_INPUT_PINS>::value;
}

static bool
read_input_pin(uint32_t pin)
{
	bool r = stm32_gpioread(pin);
#if not GPIO_KBD_IN_ACTIVE_HIGH
	r = not r;
#endif
	return r;
}

static void
tick(void *)
{
	add_gpio_pins(state, read_input_port(), GPIO_KBD_INPUT_PINS);
	switch_output_line();
	if (output_n == 0)
	{
		add_bool(state, read_input_pin(GPIO_KBD_IN_POWER));
		start_next_cycle(state);
		put(buffer, bounce_filter(state.pressed_keys_mask));
	}
}

static inline void
start()
{
	// Init output_n with last line to turn on the first one then.
	output_n = GPIO_KBD_N_OUTPUTS - 1;
	switch_output_line();
	start_next_cycle(state);
}

inline void
hrt_start()
{
	start();
	hrt_call_every(&hrt_scan_entry,
			KBD_SCAN_INTERVAL_usec, KBD_SCAN_INTERVAL_usec,
			tick, nullptr);
}

} // end of namespace scan_gpios

// Functions

static ssize_t
read(FAR struct file *filp, FAR char *buf, size_t buf_size)
{
	buf_size /= sizeof(pressed_mask_t);
	pressed_mask_t * first = reinterpret_cast<pressed_mask_t*>(buf);
	pressed_mask_t * last = copy_n_reverse_from(buffer, first, buf_size);
	return (last - first) * sizeof(pressed_mask_t);
}

inline bool
register_device()
{
	static const file_operations kbd_ops = {
		0, // open
		0, // close
		read,
		0, // write
		0, // seek
		0, // ioctl
#ifndef CONFIG_DISABLE_POLL
		0, // poll
#endif
	};

	int r = register_driver(KBD_DEVICE_PATH, &kbd_ops, 0666, nullptr);
	if (r < 0)
	{
		message("Failed to register " KBD_DEVICE_PATH " driver: %s\n",
			strerror(-r));
	}
	return r == 0;
}

} // end of namespace airleash_kbd

void __EXPORT
airleash_kbd_gpioconfig()
{
	stm32_configgpio(GPIO_KBD_IN_POWER);
	stm32_configgpio(GPIO_KBD_IN_0);
	stm32_configgpio(GPIO_KBD_IN_1);
	stm32_configgpio(GPIO_KBD_IN_2);
	stm32_configgpio(GPIO_KBD_OUT_0);
	stm32_configgpio(GPIO_KBD_OUT_1);
	stm32_configgpio(GPIO_KBD_OUT_2);
}

void __EXPORT
airleash_kbd_start()
{
	using namespace airleash_kbd;
	bool ok = register_device();
	if (ok) { scan_gpios::hrt_start(); }
}
