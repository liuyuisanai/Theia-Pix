#pragma once

#include <type_traits>

#include <drivers/drv_airleash.h>

#include "kbd_defines.hpp"

namespace kbd_handler {

using namespace airleash;

constexpr bool
in_air_mode(ModeId m)
{
	return m >= ModeId::FLIGHT;
}


template <ModeId, EventKind, ButtonId, typename /* enable_if */ = void>
struct handle : Default // Only this one should be inherited from Default.
{ static void exec(App&) { say("unknown"); } }; // TODO tone bzzz

/*
 * Power long press hangling is special one.
 */

template <ModeId AnyMode>
struct handle<AnyMode, EventKind::LONG_PRESS, BTN_MASK_POWER
	//, typename std::enable_if<AnyMode != ModeId::CONFIRM_ARM>::type
>
{
	static void
	exec(App&)
	{
		say("power off");
		usleep(5000); // 5ms to make the message visible at console.
		halt_and_power_off();
	}
};

/*
 * State transitions
 */

template <>
struct handle<ModeId::PREFLIGHT, EventKind::LONG_PRESS, BTN_MASK_PLAY>
{
	static void
	exec(App & app)
	{
		if (app.drone_status.ready_to_arm())
		{
			// notify drone
			app.set_mode_transition(ModeId::CONFIRM_ARM);
			say("ARM waiting confirm.");
		}
		else
		{
			app.tone.arm_failed();
			say("ARM request failed: drone is not ready to arm");
		}
	}
};

template <>
struct handle<ModeId::CONFIRM_ARM, EventKind::LONG_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		if (app.drone_status.ready_to_arm())
		{
			app.drone_cmd.send_arm_command(app.drone_status);
		}
		else
		{
			app.tone.arm_failed();
			say("ARM confirm failed: drone is not ready to arm");
		}
	}
};

template <EventKind EVENT, ButtonId BUTTON>
struct handle<ModeId::CONFIRM_ARM, EVENT, BUTTON,
	typename std::enable_if<EVENT == EventKind::RELEASE or EVENT == EventKind::TIMEOUT>::type
> {

//: Default // LONG_PRESS/REPEAT_PRESS does not matter
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::PREFLIGHT);
		say("Switching back to PREFLIGHT mode.");
	}
};

template <ModeId AnyMode>
struct handle<AnyMode, EventKind::SHORT_PRESS, BTN_MASK_POWER,
	typename std::enable_if<AnyMode != ModeId::CONFIRM_ARM>::type
> {
	static void
	exec(App&)
	{
		say("open menu");
		// app.set_mode_transition(ModeId::MENU);
	}
};

template <>
struct handle<ModeId::FLIGHT, EventKind::SHORT_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::FLIGHT_ALT);
	}
};

template <>
struct handle<ModeId::FLIGHT_ALT, EventKind::SHORT_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::FLIGHT);
	}
};

template <ButtonId AnyButton>
struct handle<ModeId::FLIGHT_ALT, EventKind::TIMEOUT, AnyButton>
{
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::FLIGHT);
	}
};

template <>
struct handle<ModeId::FLIGHT, EventKind::LONG_PRESS, BTN_MASK_CENTER>
{
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::SHORTCUT);
	}
};

template <EventKind EVENT, ButtonId AnyButton>
struct handle<ModeId::SHORTCUT, EVENT, AnyButton,
	typename std::enable_if<
		EVENT == EventKind::RELEASE or EVENT == EventKind::TIMEOUT
	>::type
> {
	static void
	exec(App & app)
	{
		app.set_mode_transition(ModeId::FLIGHT);
	}
};

} // end of namespace kbd_handler

#include "kbd_handler_flight.hpp"
#include "kbd_handler_copter.hpp"
#include "kbd_handler_base.hpp"  /* this one should be last */
