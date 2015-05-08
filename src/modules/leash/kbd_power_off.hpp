#pragma once

#include <drivers/drv_airleash.h>

#include "debug.hpp"
#include "kbd_defines.hpp"
#include "kbd_handler_prolog.hpp"

namespace kbd_handler {

/*
 * Power long press hangling is special one.
 */

template <ModeId MODE>
struct handle<MODE, EventKind::LONG_KEYPRESS, BTN_MASK_POWER, When<
	mode_allows_power_off(MODE)
> > {
	static void
	exec(App&)
	{
		say("power off");
		usleep(5000); // 5ms to make the message visible at console.
		halt_and_power_off();
	}
};

} // end of namespace kbd_handler
