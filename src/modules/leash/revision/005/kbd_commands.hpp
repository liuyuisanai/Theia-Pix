#pragma once

#include <uORB/topics/vehicle_command.h>
#include <sys/ioctl.h>

#include "../../debug.hpp"
#include "../../kbd_defines.hpp"
#include "../../kbd_handler_prolog.hpp"

namespace kbd_handler
{

#define _BLUETOOTH21_BASE		0x2d00

#define PAIRING_ON			_IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF			_IOC(_BLUETOOTH21_BASE, 1)
#define PAIRING_TOGGLE		_IOC(_BLUETOOTH21_BASE, 2)

/*
 * PLAY button.
 */

template <ModeId MODE>
struct handle<MODE, EventKind::SHORT_KEYPRESS, BTN_MASK_PLAY, When<
    MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT or MODE == ModeId::SHORTCUT
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT SHORT_KEYPRESS PLAY");
		app.drone_cmd.send_command(REMOTE_CMD_PLAY_PAUSE);
	}
};

template <ModeId MODE>
struct handle<MODE, EventKind::SHORT_KEYPRESS, BTN_MASK_TO_H, When<
	MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT
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
	MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT
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

//template <>
//struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_LEFT>
//{
//	static void
//	exec(App & app)
//	{
//		say("SHORTCUT SHORT_LEFT");
//		app.drone_cmd.send_command(REMOTE_CMD_SET_FIRST_POINT);
//	}
//};
//
//template <>
//struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_RIGHT>
//{
//	static void
//	exec(App & app)
//	{
//		say("SHORTCUT SHORT_RIGHT");
//		app.drone_cmd.send_command(REMOTE_CMD_SET_SECOND_POINT);
//	}
//};
//
//template <>
//struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_POWER>
//{
//	static void
//	exec(App & app)
//	{
//		say("SHORTCUT SHORT_KEYPRESS POWER");
//		app.drone_cmd.send_command(REMOTE_CMD_CLEAR_POINTS);
//	}
//};


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

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_ALT, EVENT, BTN_MASK_LEFT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("F");
		app.drone_cmd.send_command(REMOTE_CMD_LEFT);
	}
};

template <EventKind EVENT>
struct handle< ModeId::FLIGHT_ALT, EVENT, BTN_MASK_RIGHT, When<
	event_is_short_or_repeat_press(EVENT)
> > {
	static void
	exec(App & app)
	{
		say("F");
		app.drone_cmd.send_command(REMOTE_CMD_RIGHT);
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

template <ModeId MODE>
struct handle<MODE, EventKind::SHORT_KEYPRESS, BTN_MASK_TO_ME, When<
    MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT
> >
{
	static void
	exec(App & app)
	{
		say("Send come to me");
		app.drone_cmd.send_come_to_me_command();
	}
};

} // end of namespace kbd_handler
