#pragma once

#include <drivers/drv_airleash.h>

#include "kbd_defines.hpp"
#include "uorb_functions.hpp"

namespace kbd_handler {

using namespace airleash;

static void
say(const char s[])
{ fprintf(stderr, "%s\n", s); }


static void
handle_default(state_t) { say("unknown"); }


static void
handle(state_t, ModeA, LongPress, Power  )
{
	say("ModeA LongPress Power  ");
	usleep(5000); // 5ms pause to make the message visible at console.
	halt_and_power_off();
}

static void
handle(state_t, ModeA, ShortPress, Power  )
{
	say("ModeA ShortPress Power  ");

	// pmenu_ctrl->open();
}

static void
handle(state_t, ModeA, ShortPress, Play   )
{
	say("ModeA ShortPress Play   ");
	if (is_drone_active())
	{
		if (is_drone_armed())
			send_command(REMOTE_CMD_PLAY_PAUSE);
		else
			send_arm_command();
	}
}

static void
handle(state_t, ModeA, ShortPress, Up     )
{
	say("ModeA ShortPress Up     ");
	send_command(REMOTE_CMD_FURTHER);
}

static void
handle(state_t, ModeA, ShortPress, Down   )
{
	say("ModeA ShortPress Down   ");
	send_command(REMOTE_CMD_CLOSER);
}

// static void
// handle(state_t, ModeA, ShortPress, Center )
// {
//        	say("ModeA ShortPress Center ");
// }

static void
handle(state_t, ModeA, ShortPress, Left   )
{
	say("ModeA ShortPress Left   ");
	send_command(REMOTE_CMD_LEFT);
}

static void
handle(state_t, ModeA, ShortPress, Right  )
{
	say("ModeA ShortPress Right  ");
	send_command(REMOTE_CMD_RIGHT);
}

} // end of namespace kbd_handler
