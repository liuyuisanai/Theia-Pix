#include <nuttx/config.h>
#include <nuttx/fs/fs.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <stm32.h>

#include <board_config.h>
#include <drivers/drv_frame_button.h>
#include <drivers/drv_hrt.h>

#include <uORB/uORB.h>
#include <uORB/topics/frame_button.h>

#include "bounce_filter.hpp"
#include "buffer.hpp"
#include "config_helper.hpp"
#include "driver.h"
#include "state.hpp"
#include "stm32_helper.hpp"
#include "button_state.hpp"

#define message(...)	printf(__VA_ARGS__)

namespace frame_kbd {


// Buffer definition
using buffer_size_t = uint8_t;
static_assert(buffer_size_t(FRAME_BUTT_SCAN_BUFFER_N_ITEMS) * 2 != 0,
		"buffer_size_t is too small.");

using KbdButtonState = ButtonState<
	pressed_mask_t, FRAME_BUTT_SCAN_BUFFER_N_ITEMS,
	hrt_abstime, FRAME_BUTT_SCAN_INTERVAL_usec
>;

using buffer_t = AlwaysFullBuffer<
	pressed_mask_t,
	buffer_size_t,
	FRAME_BUTT_SCAN_BUFFER_N_ITEMS * 2,
	volatile buffer_size_t
>;

// Globals
static buffer_t buffer;

// Functions

static ssize_t
read(pressed_mask_t *buf, size_t buf_size)
{
	buf_size /= sizeof(pressed_mask_t);
	pressed_mask_t * first = reinterpret_cast<pressed_mask_t*>(buf);
	pressed_mask_t * last = copy_n_reverse_from(buffer, first, buf_size);
	return (last - first) * sizeof(pressed_mask_t);
}

namespace scan_gpios {

static struct hrt_call               hrt_scan_entry;
static ScanState                     state;
static BounceFilter<pressed_mask_t>  bounce_filter;
static KbdButtonState                btn, last_btn;
static press_type                    event;

static orb_advert_t		             frame_button_pub;
static frame_button_s                data;

static bool
read_input_pin(uint32_t pin)
{
	bool r = stm32_gpioread(pin);
#if not GPIO_KBD_IN_ACTIVE_HIGH
	r = not r;
#endif
	return r;
}

void
update_buttons(KbdButtonState & s, hrt_abstime now)
{
	pressed_mask_t masks[FRAME_BUTT_SCAN_BUFFER_N_ITEMS];
	// f_kbd should always be full enough.  Ignore read result.
    read(masks, sizeof(masks));
	s.update(now, masks);
}

static void event_printer( press_type press) {
//#define FULL_PRINT
    switch (press) {
        case SHORT_KEYPRESS:
            DOG_PRINT("[frame button] event SHORT_KEYPRESS\n");
            break;
        case LONG_KEYPRESS:
            DOG_PRINT("[frame button] event LONG_KEYPRESS\n");
            break;
#ifdef FULL_PRINT
        case SUSPECT_FOR_DOUBLE:
            DOG_PRINT("[frame button] event SUSPECT_FOR_DOUBLE\n");
            break;
        case SUSPECT_FOR_TRIPLE:
            DOG_PRINT("[frame button] event SUSPECT_FOR_TRIPLE\n");
            break;
#endif
        case DOUBLE_CLICK:
            DOG_PRINT("[frame button] event DOUBLE_CLICK\n");
            break;
        case TRIPLE_CLICK:
            DOG_PRINT("[frame button] event TRIPLE_CLICK\n");
            break;
    }
    fflush(stderr);
}


void
post_result(press_type event, hrt_abstime timestamp)
{
    switch (event) {
        case SHORT_KEYPRESS:
            data.state = button_state::SINGLE_CLICK;
            break;
        case DOUBLE_CLICK:
            data.state = button_state::DOUBLE_CLICK;
            break;
        case TRIPLE_CLICK:
            data.state = button_state::TRIPLE_CLICK;
            break;
        case LONG_KEYPRESS:
            data.state = button_state::LONG_KEYPRESS;
            break;
        default:
            return;
            break;
    }
    data.timestamp = timestamp;
    if (frame_button_pub > 0) {
        orb_publish(ORB_ID(frame_button_state), frame_button_pub, &data);
    } else {
        DOG_PRINT("[ERROR frame button] frame_button_pub not advertised %d\n", frame_button_pub);
    }
}

static void
tick(void *)
{
    add_bool(state, read_input_pin(FC_BUTT_BGC));
    start_next_cycle(state);
    put(buffer, bounce_filter(state.pressed_keys_mask));

    hrt_abstime now = hrt_absolute_time();
    update_buttons(btn, now);
    event = handle_kbd_state(btn, last_btn, now, event);
    event_printer(event);
    post_result(event, now);
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
    frame_button_pub = orb_advertise(ORB_ID(frame_button_state), &data);
	start();
	hrt_call_every(&hrt_scan_entry,
			FRAME_BUTT_SCAN_INTERVAL_usec, FRAME_BUTT_SCAN_INTERVAL_usec,
			tick, nullptr);
}

} // end of namespace scan_gpios


} // end of namespace frame_kbd


void __EXPORT
frame_button_start()
{
	using namespace frame_kbd;
	scan_gpios::hrt_start();
}

/*
  driver entry point
 */
int main(int argc, const char *argv[])
{
	const char *verb = argv[1];

	/*
	 * Start/load the driver.
	 */
	if (!strcmp(verb, "start")) {
            frame_button_start();
	}

	return 0;
}
