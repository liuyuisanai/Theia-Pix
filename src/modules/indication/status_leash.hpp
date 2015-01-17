#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Only STM32 supported.
#endif

#include <cstdint>
#include <cstdio>
#include <ctime>

#include <board_leds.h>
#include <drivers/drv_hrt.h>
#include <systemlib/param/param.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_status.h>

#include "leds.hpp"

namespace indication { namespace status {

static int status_sub;
static int airdog_sub;

static param_t param_use_blue_led;
static int32_t use_blue_led;

static bool pos_valid, link_valid, force_update;

void
init()
{
	status_sub = orb_subscribe(ORB_ID(vehicle_status));
	airdog_sub = orb_subscribe(ORB_ID(airdog_status));
	param_use_blue_led = param_find("LEASH_USE_BLUE_LED");
	force_update = true;
	pos_valid = false;
	link_valid = false;
	use_blue_led = -1;
}

void
update(hrt_abstime now)
{
	vehicle_status_s status;
	orb_copy(ORB_ID(vehicle_status), status_sub, &status);

	airdog_status_s airdog_status;
	orb_copy(ORB_ID(airdog_status), airdog_sub, &airdog_status);

	bool changed_pos_valid = pos_valid != status.condition_global_position_valid;
	pos_valid = status.condition_global_position_valid;

	bool link_ok = airdog_status.timestamp > 0
			and now - airdog_status.timestamp < 3000000;
	bool changed_link_valid = link_valid != link_ok;
	link_valid = link_ok;

	int32_t x;
	param_get(param_use_blue_led, &x);
	bool changed_use_blue_led = x != use_blue_led;
	use_blue_led = x;


	if (force_update or changed_link_valid or changed_pos_valid or changed_use_blue_led)
	{
		uint32_t pattern;

		if (use_blue_led) {
			pattern = pos_valid ? 0xFFFFFFFe : 0x80000000;
			leds::set_pattern_repeat(LED_STATUS, pattern);

			pattern = link_valid ? 0xFFFFFFFd : 0x80000000;
			leds::set_pattern_repeat(LED_LEASHED, pattern);
		}
		else
		{
			if (pos_valid and link_valid)
				pattern = 0xFFFFFFFe;
			else if (pos_valid or link_valid)
				pattern = 0x88000000;
			else
				pattern = 0x80000000;
			printf("pattern %x\n", pattern);
			leds::set_pattern_repeat(LED_STATUS, pattern);
			leds::set_pattern_repeat(LED_LEASHED, 0xFFFFFFFF);
		}
	}

	force_update = false;
}

void
done()
{ orb_unsubscribe(status_sub); }


}} // end of namespace indication::leds
