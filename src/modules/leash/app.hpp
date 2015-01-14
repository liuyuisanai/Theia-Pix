#pragma once

#include "kbd_defines.hpp"
#include "tones.hpp"

struct App;

namespace kbd_handler
{
template <EventKind>
void
handle_event(App &, ModeId m, ButtonId b);
} // end of namespace kbd_handler

struct App
{
	using ModeId = kbd_handler::ModeId;
	using ButtonId = kbd_handler::ButtonId;
	using EventKind = kbd_handler::EventKind;
	using timestamp_type = hrt_abstime;

	ModeId mode, next_mode;
	timestamp_type stamp;
	Tone tone;

	App() : mode(ModeId::A) {}

	void
	set_mode_next(ModeId m)
	{
		// play tone
		next_mode = m;
	}

	template <EventKind EVENT>
	void
	handle_button(ButtonId btn)
	{
		tone.key_press();
		kbd_handler::handle_event<EVENT>(*this, mode, btn);
		if (next_mode != mode and next_mode != ModeId::NONE) {
			// FIXME is NONE required?
			mode = next_mode;
		}
	}

	void
	handle_time(timestamp_type now)
	{
		// TODO switch to default state;
	}
};
