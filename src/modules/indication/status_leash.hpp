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
#include <modules/leash/kbd_defines.hpp>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/leash_status.h>

#include "leds.hpp"

using namespace kbd_handler;
namespace indication { namespace status {

static int status_sub;
static int airdog_sub;
static int leash_sub;

static param_t param_use_blue_led;
static int32_t use_blue_led;

static bool pos_valid, link_valid, force_update;
static bool loiter_mode;
leash_status_s l_status;

void
init()
{
	status_sub = orb_subscribe(ORB_ID(vehicle_status));
	airdog_sub = orb_subscribe(ORB_ID(airdog_status));
    leash_sub = orb_subscribe(ORB_ID(leash_status));
	param_use_blue_led = param_find("LEASH_USE_BLUE_LED");
	force_update = true;
	pos_valid = false;
	link_valid = false;
	loiter_mode = false;
	use_blue_led = -1;
}

void
update(hrt_abstime now)
{
	vehicle_status_s status;
	orb_copy(ORB_ID(vehicle_status), status_sub, &status);

	airdog_status_s airdog_status;
	orb_copy(ORB_ID(airdog_status), airdog_sub, &airdog_status);


	bool led_updated = false;
	orb_check(leash_sub, &led_updated);
    if (led_updated) {
        orb_copy(ORB_ID(leash_status), leash_sub, &l_status);
    }


	bool changed_pos_valid = pos_valid != status.condition_global_position_valid;
	pos_valid = status.condition_global_position_valid;

	bool link_ok = airdog_status.timestamp > 0
			and now - airdog_status.timestamp < 3000000;
	bool changed_link_valid = link_valid != link_ok;
	link_valid = link_ok;

	bool now_loiter = link_valid
			and (airdog_status.state_main == MAIN_STATE_LOITER);
	bool changed_mode = loiter_mode != now_loiter;
	loiter_mode = now_loiter;
	if (changed_mode)
		fprintf(stderr, "State main %i %i\n", airdog_status.state_main, loiter_mode);

	int32_t x;
	param_get(param_use_blue_led, &x);
	bool changed_use_blue_led = x != use_blue_led;
	use_blue_led = x;


	if (force_update or changed_use_blue_led
	or changed_link_valid or changed_pos_valid or changed_mode or led_updated
	) {
        uint32_t pattern;

		if (use_blue_led) {
            switch((ModeId) l_status.menu_mode) {
                case ModeId::FLIGHT:
                    pattern = pos_valid ? 0x1000 : 0xFFFFe;
                    break;
                case ModeId::FLIGHT_ALT:
                    pattern = pos_valid ? 0x5000 : 0xFFFFa;
                    break;
                case ModeId::FLIGHT_CAM:
                    pattern = pos_valid ? 0x1500 : 0xFFFea;
                    break;
                default:
                    pattern = pos_valid ? 0x80000000 : 0xFFFF7;
                    break;
            }
			leds::set_pattern_repeat(LED_STATUS, pattern);

			if (link_valid)
				pattern = loiter_mode ? 0xFF00FF00 : 0xFFFFFFFd;
			else
				pattern = 0x80000000;

			leds::set_pattern_repeat(LED_LEASHED, pattern);
		}
		else
		{
			printf("pattern %x\n", pattern);
            switch((ModeId) l_status.menu_mode) {
                case ModeId::FLIGHT:
                    pattern = 0xFFFFFFFe;
                    break;
                case ModeId::FLIGHT_ALT:
                    pattern = 0xFFFFFFFa;
                    break;
                case ModeId::FLIGHT_CAM:
                    pattern = 0xFFFFFFea;
                    break;
                default:
                    pattern = 0x15000000;
                    break;
            }
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
