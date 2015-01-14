#pragma once

#include <drivers/drv_airleash.h>

#include "kbd_defines.hpp"
#include "uorb_functions.hpp"

namespace kbd_handler {

using namespace airleash;

static void
say(const char s[])
{ fprintf(stderr, "%s\n", s); }

template <ModeId, EventKind, ButtonId>
struct handle : Default // Only this one should be inherited from Default.
{ static void exec(App&) { say("unknown"); } };

template <ModeId AnyMode>
struct handle<AnyMode, EventKind::LONG_PRESS, BTN_MASK_POWER>
{
	static void
	exec(App&)
	{
		say("EventKind::LONG_PRESS Power  ");
		usleep(5000); // 5ms to make the message visible at console.
		halt_and_power_off();
	}
};

//template <>
//struct handle<ModeId::A, RepeatPress, Power>
//{
//	static void
//	exec(App&)
//	{
//		say("ModeId::A RepeatPress Power  ");
//	}
//};

template <ModeId AnyMode>
struct handle<AnyMode, EventKind::SHORT_PRESS, BTN_MASK_POWER>
{
	static void
	exec(App&)
	{
		say("EventKind::SHORT_PRESS Power  ");
		// pmenu_ctrl->open();
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_PLAY>
{
	static void
	exec(App&)
	{
		say("ModeId::A EventKind::SHORT_PRESS Play   ");
		if (is_drone_active())
		{
			if (is_drone_armed())
				send_command(REMOTE_CMD_PLAY_PAUSE);
			else
				send_arm_command();
		}
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_UP>
{
	static void
	exec(App&)
	{
		say("ModeId::A EventKind::SHORT_PRESS Up     ");
		send_command(REMOTE_CMD_FURTHER);
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_DOWN>
{
	static void
	exec(App&)
	{
		say("ModeId::A EventKind::SHORT_PRESS Down   ");
		send_command(REMOTE_CMD_CLOSER);
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		say("ModeId::A EventKind::SHORT_PRESS Center ");
		app.set_mode_next(ModeId::DRIFT);
	}
};

template <>
struct handle<ModeId::DRIFT, EventKind::SHORT_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		say("ModeId::DRIFT EventKind::SHORT_PRESS Center ");
		app.set_mode_next(ModeId::A);
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_LEFT>
{
	static void
	exec(App&)
	{
		say("ModeId::A EventKind::SHORT_PRESS Left   ");
		send_command(REMOTE_CMD_LEFT);
	}
};

template <>
struct handle<ModeId::A, EventKind::SHORT_PRESS, BTN_MASK_RIGHT>
{
	static void
	exec(App&)
	{
		say("ModeId::A EventKind::SHORT_PRESS Right  ");
		send_command(REMOTE_CMD_RIGHT);
	}
};

} // end of namespace kbd_handler
