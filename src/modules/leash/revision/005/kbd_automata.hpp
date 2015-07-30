#pragma once

#include "../../debug.hpp"
#include "../../kbd_defines.hpp"
#include "../../kbd_handler_prolog.hpp"

namespace kbd_handler {

/*
 * State transitions
 */

static void
on_mode_timeout(App & app, ModeId mode_if_inactive)
{
	ModeId m = mode_if_inactive;
	if (app.drone_status.active())
	{
		m = app.drone_status.in_air() ?
			ModeId::FLIGHT : ModeId::PREFLIGHT;
	}
	app.set_mode_transition(m);
}

template <ModeId MODE>
struct handle< MODE, EventKind::COPTER_CHANGED_STATE, BTN_NONE, When<
        MODE == ModeId::INIT or MODE == ModeId::PREFLIGHT
> > {
	static void
	exec(App & app)
        {
                say("INIT or PREFLIGHT on_mode_timeout");
		on_mode_timeout(app, ModeId::INIT);
	}
};

template <ModeId MODE>
struct handle< MODE, EventKind::COPTER_CHANGED_STATE, BTN_NONE, When<
	in_air_mode(MODE)
> > {
	static void
	exec(App & app)
	{
		say("FLIGHT_* on_mode_timeout");
		on_mode_timeout(app, ModeId::FLIGHT_NO_SIGNAL);
	}
};

//template <>
//struct handle<ModeId::FLIGHT_ALT, EventKind::KEY_TIMEOUT, BTN_NONE>
//{
//	static void
//	exec(App & app)
//	{
//		say("FLIGHT_ALT KEY_TIMEOUT");
//		app.set_mode_transition(ModeId::FLIGHT);
//	}
//};

template <>
struct handle< ModeId::FLIGHT, EventKind::LONG_KEYPRESS, BTN_MASK_MODE>
{
	static void
	exec(App & app)
	{
		say("FLIGHT LONG_KEYPRESS MODE");
		app.set_mode_transition(ModeId::FLIGHT_ALT);
	}
};

template <>
struct handle< ModeId::FLIGHT_ALT, EventKind::SHORT_KEYPRESS, BTN_MASK_MODE>
{
	static void
	exec(App & app)
	{
		say("FLIGHT LONG_KEYPRESS MODE");
		app.set_mode_transition(ModeId::FLIGHT);
	}
};

} // end of namespace kbd_handler
