#pragma once

#include <type_traits>

#include <uORB/topics/vehicle_command.h>

#include "kbd_defines.hpp"

namespace kbd_handler {

using namespace airleash;

constexpr bool
event_is_short_or_repeat_press(EventKind E)
{ return E == EventKind::SHORT_KEYPRESS or E == EventKind::REPEAT_KEYPRESS; }

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
struct handle<MODE, EventKind::LONG_KEYPRESS, BTN_MASK_PLAY, When<
	MODE == ModeId::FLIGHT or MODE == ModeId::FLIGHT_ALT
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT or FLIGHT_ALT LONG_KEYPRESS PLAY");
		if (app.drone_status.in_air())
			app.drone_cmd.send_command(REMOTE_CMD_LAND_DISARM);
	}
};

template <>
struct handle<ModeId::SHORTCUT, EventKind::LONG_KEYPRESS, BTN_MASK_PLAY>
{
	static void
	exec(App & app)
	{
		say("SHORTCUT LONG_KEYPRESS PLAY");
		app.drone_cmd.send_command(REMOTE_CMD_LAND_DISARM);
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
		app.drone_cmd.send_command(REMOTE_CMD_FURTHER);
	}
};

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_KEYPRESS, BTN_MASK_DOWN>
{
	static void
	exec(App & app)
	{
		say("B");
		app.drone_cmd.send_command(REMOTE_CMD_CLOSER);
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
 * SHORTCUT mode.
 */

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_KEYPRESS, BTN_MASK_UP>
{
	static void
	exec(App & app)
	{
		say("G");
		app.drone_cmd.send_command(REMOTE_CMD_COME_TO_ME);
	}
};

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
