#pragma once

#include <type_traits>

#include "kbd_defines.hpp"

namespace kbd_handler {

template <ButtonId AnyButton>
struct handle<ModeId::PREFLIGHT, EventKind::COPTER_CHANGED_STATE, AnyButton>
{
	static void
	exec(App & app)
	{
		app.set_mode_transition(app.drone_status.in_air() ?
					ModeId::FLIGHT : ModeId::PREFLIGHT);
	}
};

template <ModeId MODE, ButtonId AnyButton>
struct handle<MODE, EventKind::COPTER_CHANGED_STATE, AnyButton,
	typename std::enable_if< in_air_mode(MODE) >::type
> {
	static void
	exec(App & app)
	{
		if (not app.drone_status.in_air())
			app.set_mode_transition(ModeId::PREFLIGHT);
	}
};

} // end of namespace kbd_handler
