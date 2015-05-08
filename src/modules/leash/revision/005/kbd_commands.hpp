#pragma once

#include <uORB/topics/vehicle_command.h>

#include "../../debug.hpp"
#include "../../kbd_defines.hpp"
#include "../../kbd_handler_prolog.hpp"

namespace kbd_handler
{

/*
 * PLAY button.
 */

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_KEYPRESS, BTN_MASK_PLAY>
{
	static void
	exec(App & app)
	{
		say("FLIGHT SHORT_KEYPRESS PLAY");
		app.drone_cmd.send_command(REMOTE_CMD_PLAY_PAUSE);
	}
};

template <ModeId MODE>
struct handle<MODE, EventKind::SHORT_KEYPRESS, BTN_MASK_TO_H, When<
	MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT or MODE == ModeId::FLIGHT_CAM
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT/FLIGHT_ALT/FLIGHT_CAM SHORT_KEYPRESS (H)");
		if (app.drone_status.in_air())
			app.drone_cmd.send_command(REMOTE_CMD_LAND_DISARM);
	}
};

template <ModeId MODE>
struct handle<MODE, EventKind::LONG_KEYPRESS, BTN_MASK_TO_H, When<
	MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT or MODE == ModeId::FLIGHT_CAM
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT/FLIGHT_ALT/FLIGHT_CAM LONG_KEYPRESS (H)");
		app.drone_cmd.send_rtl_command(app.drone_status);
	}
};

/*
 * Follow Line: LEFT/RIGHT and POWER button.
 */

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_LEFT>
{
	static void
	exec(App & app)
	{
		say("SHORTCUT SHORT_LEFT");
		app.drone_cmd.send_command(REMOTE_CMD_SET_FIRST_POINT);
	}
};

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_RIGHT>
{
	static void
	exec(App & app)
	{
		say("SHORTCUT SHORT_RIGHT");
		app.drone_cmd.send_command(REMOTE_CMD_SET_SECOND_POINT);
	}
};

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_POWER>
{
	static void
	exec(App & app)
	{
		say("SHORTCUT SHORT_KEYPRESS POWER");
		app.drone_cmd.send_command(REMOTE_CMD_CLEAR_POINTS);
	}
};


/*
 * FLIGHT mode.
 */

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_KEYPRESS, BTN_MASK_UP>
{
	static void
	exec(App & app)
	{
		say("A");
		app.drone_cmd.send_command(REMOTE_CMD_UP);
	}
};

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_KEYPRESS, BTN_MASK_DOWN>
{
	static void
	exec(App & app)
	{
		say("B");
		app.drone_cmd.send_command(REMOTE_CMD_DOWN);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT, EVENT, BTN_MASK_LEFT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("C");
		app.drone_cmd.send_command(REMOTE_CMD_LEFT);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT, EVENT, BTN_MASK_RIGHT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("D");
		app.drone_cmd.send_command(REMOTE_CMD_RIGHT);
	}
};


/*
 * FLIGHT_ALT mode.
 */

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_ALT, EVENT, BTN_MASK_UP, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("E");
		app.drone_cmd.send_command(REMOTE_CMD_FURTHER);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_ALT, EVENT, BTN_MASK_DOWN, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("F");
		app.drone_cmd.send_command(REMOTE_CMD_CLOSER);
	}
};

/*
 * FLIGHT_CAM mode.
 */

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_CAM, EVENT, BTN_MASK_UP, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("Camera up");
		app.drone_cmd.send_command(REMOTE_CMD_CAM_UP);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_CAM, EVENT, BTN_MASK_DOWN, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("Camera down");
		app.drone_cmd.send_command(REMOTE_CMD_CAM_DOWN);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_CAM, EVENT, BTN_MASK_LEFT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("Camera left");
		app.drone_cmd.send_command(REMOTE_CMD_CAM_LEFT);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_CAM, EVENT, BTN_MASK_RIGHT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("Camera right");
		app.drone_cmd.send_command(REMOTE_CMD_CAM_RIGHT);
	}
};

template <>
struct handle<ModeId::FLIGHT_CAM, EventKind::LONG_KEYPRESS, BTN_MASK_POWER>
{
	static void
	exec(App & app)
	{
		say("FLIGHT_CAM LONG_KEYPRESS OK");
		app.drone_cmd.send_command(REMOTE_CMD_CAM_RESET);
	}
};

/*
 * FLIGHT_ALT and FLIGHT_CAM timeout handle
 */
template <ModeId MODE, ButtonId BUTTON>
struct handle< MODE, EventKind::KEY_RELEASE, BUTTON, When<
    BUTTON != BTN_MASK_MODE
	and (MODE == ModeId::FLIGHT_ALT or MODE == ModeId::FLIGHT_CAM)
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT_ALT/FLIGHT_CAM Restart");
		app.restart_key_timeout();
	}
};

template <ModeId MODE>
struct handle<MODE, EventKind::SHORT_KEYPRESS, BTN_MASK_TO_ME, When<
    MODE == ModeId::FLIGHT_ALT or MODE == ModeId::FLIGHT_CAM or MODE == ModeId::FLIGHT_CAM
> >
{
	static void
	exec(App & app)
	{
		say("Send come to me");
		app.drone_cmd.send_command(REMOTE_CMD_COME_TO_ME);
	}
};

/*
 * SHORTCUT mode.
 */

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_DOWN>
{
	static void
	exec(App & app)
	{
		say("H");
		app.drone_cmd.send_command(REMOTE_CMD_LOOK_DOWN);
	}
};

} // end of namespace kbd_handler
