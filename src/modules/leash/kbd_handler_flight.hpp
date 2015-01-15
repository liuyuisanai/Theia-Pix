#pragma once

#include <type_traits>

#include <uORB/topics/vehicle_command.h>

#include "kbd_defines.hpp"

namespace kbd_handler {

using namespace airleash;

constexpr bool
is_short_or_repeat_press(EventKind E)
{ return E == EventKind::SHORT_PRESS or E == EventKind::REPEAT_PRESS; }

template <EventKind E>
using enable_if_short_or_repeat_press = typename std::enable_if<is_short_or_repeat_press(E)>::type;


/*
 * PLAY button.
 */

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_PRESS, BTN_MASK_PLAY>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_PLAY_PAUSE);
	}
};

template <ModeId M>
struct handle<M, EventKind::LONG_PRESS, BTN_MASK_PLAY,
	typename std::enable_if<
		M == ModeId::FLIGHT or M == ModeId::FLIGHT_ALT
	>::type
> {
	static void
	exec(App & app)
	{
		if (app.drone_status.in_air())
			app.drone_cmd.send_command(REMOTE_CMD_LAND_DISARM);
	}
};

template <>
struct handle<ModeId::SHORTCUT, EventKind::LONG_PRESS, BTN_MASK_PLAY>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_LAND_DISARM);
	}
};


/*
 * FLIGHT mode.
 */

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_PRESS, BTN_MASK_UP>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_FURTHER);
	}
};

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_PRESS, BTN_MASK_DOWN>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_CLOSER);
	}
};

template <EventKind E>
struct handle<
	ModeId::FLIGHT, E, BTN_MASK_LEFT,
	enable_if_short_or_repeat_press<E>
> {
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_LEFT);
	}
};

template <EventKind E>
struct handle<
	ModeId::FLIGHT, E, BTN_MASK_RIGHT,
	enable_if_short_or_repeat_press<E>
> {
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_RIGHT);
	}
};


/*
 * FLIGHT_ALT mode.
 */

template <EventKind EVENT>
struct handle<
	ModeId::FLIGHT_ALT, EVENT, BTN_MASK_UP,
	enable_if_short_or_repeat_press<EVENT>
> {
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_FURTHER);
	}
};

template <EventKind EVENT>
struct handle<
	ModeId::FLIGHT_ALT, EVENT, BTN_MASK_DOWN,
	enable_if_short_or_repeat_press<EVENT>
> {
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_CLOSER);
	}
};


/*
 * SHORTCUT mode.
 */

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_PRESS, BTN_MASK_UP>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_COME_TO_ME);
	}
};

template <>
struct handle<ModeId::SHORTCUT, EventKind::SHORT_PRESS, BTN_MASK_DOWN>
{
	static void
	exec(App & app)
	{
		app.drone_cmd.send_command(REMOTE_CMD_LOOK_DOWN);
	}
};

} // end of namespace kbd_handler
