#include "kbd_defines.hpp"
#include "uorb_functions.hpp"

namespace kbd_handler {

static void
say(const char s[])
{ fprintf(stderr, "%s\n", s); }


void handle_default() { say("unknown"); }


static void handle(ModeA, ShortPress, Power  )
{
	say("ModeA ShortPress Power  ");

	// pmenu_ctrl->open();
}

static void handle(ModeA, ShortPress, Play   )
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

static void handle(ModeA, ShortPress, Up     )
{
	say("ModeA ShortPress Up     ");
	send_command(REMOTE_CMD_FURTHER);
}

static void handle(ModeA, ShortPress, Down   )
{
	say("ModeA ShortPress Down   ");
	send_command(REMOTE_CMD_CLOSER);
}

// static void handle(ModeA, ShortPress, Center )
// {
//        	say("ModeA ShortPress Center ");
// }

static void handle(ModeA, ShortPress, Left   )
{
	say("ModeA ShortPress Left   ");
	send_command(REMOTE_CMD_LEFT);
}

static void handle(ModeA, ShortPress, Right  )
{
	say("ModeA ShortPress Right  ");
	send_command(REMOTE_CMD_RIGHT);
}

} // end of namespace kbd_handler
