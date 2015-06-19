#include <nuttx/config.h>
#include <nuttx/fs/fs.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <stm32.h>

#include <board_config.h>
#include <drivers/drv_frame_button.h>
#include <drivers/drv_hrt.h>

#include "bounce_filter.hpp"
#include "buffer.hpp"
#include "config_helper.hpp"
#include "driver.h"
#include "state.hpp"
#include "stm32_helper.hpp"

#define message(...)	printf(__VA_ARGS__)

namespace frame_kbd {


// Buffer definition
using buffer_size_t = uint8_t;
static_assert(buffer_size_t(FRAME_BUTT_SCAN_BUFFER_N_ITEMS) * 2 != 0,
		"buffer_size_t is too small.");
using buffer_t = AlwaysFullBuffer<
	pressed_mask_t,
	buffer_size_t,
	FRAME_BUTT_SCAN_BUFFER_N_ITEMS * 2,
	volatile buffer_size_t
>;

// Globals
static buffer_t buffer;

namespace scan_gpios {

static struct hrt_call               hrt_scan_entry;
static ScanState                     state;
static BounceFilter<pressed_mask_t>  bounce_filter;

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
    add_bool(state, read_input_pin(FC_BUTT_BGC));
    start_next_cycle(state);
    put(buffer, bounce_filter(state.pressed_keys_mask));
}

static inline void
start()
{
	start_next_cycle(state);
}

inline void
hrt_start()
{
	stm32_configgpio(FC_BUTT_BGC);
	start();
	hrt_call_every(&hrt_scan_entry,
			FRAME_BUTT_SCAN_INTERVAL_usec, FRAME_BUTT_SCAN_INTERVAL_usec,
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

	int r = register_driver(FRAME_BUTT_DEVICE_PATH, &kbd_ops, 0666, nullptr);
	if (r < 0)
	{
		message("Failed to register " FRAME_BUTT_DEVICE_PATH " driver: %s\n",
			strerror(-r));
	}
	return r == 0;
}

} // end of namespace frame_kbd


void __EXPORT
frame_button_start()
{
	using namespace frame_kbd;
	bool ok = register_device();
	if (ok) { scan_gpios::hrt_start(); }
}
