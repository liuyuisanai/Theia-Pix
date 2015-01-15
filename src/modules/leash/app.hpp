#pragma once

#include "kbd_defines.hpp"
#include "tones.hpp"
#include "uorb_functions.hpp"

struct PeriodicAction
{
	unsigned period;
	hrt_abstime next_time;

	PeriodicAction(hrt_abstime period_us)
	: period(period_us), next_time(0)
	{}

	void
	check_time(hrt_abstime now)
	{
		if (next_time <= now)
		{
			exec(now);
			next_time = now + period;
		}
	}

	virtual void
	exec(hrt_abstime) = 0;
};

struct PeriodicSayAlive : PeriodicAction
{
	unsigned n;
	PeriodicSayAlive()
	: PeriodicAction(1000000/*us*/), n(0)
	{}

	void
	exec(hrt_abstime) override {
		fprintf(stderr, "me is alive (%i)\n", n);
		++n;
	}
};

struct Timeout
{
	using ModeId = kbd_handler::ModeId;

	bool         enabled;
	hrt_abstime  future;

	Timeout() : enabled(false), future(0) {}

	void
	disable()
	{
		enabled = false;
	}

	bool
	expired(hrt_abstime now) const
	{
		return enabled and future <= now;
	}

	static bool
	define_timeout(ModeId m, hrt_abstime & duration) {
		switch (m)
		{
		// Fast timeouts
		case ModeId::CONFIRM_ARM:
			duration =  5000000u; /*  5s */
			break;
		// Slow timeouts
		case ModeId::FLIGHT_ALT:
		case ModeId::SHORTCUT:
			duration = 10000000u; /* 10s */
			break;
		// No timeout
		default:
			return false;
		}
		return true;
	}

	void
	restart(hrt_abstime now, ModeId mode)
	{
		enabled = define_timeout(mode, future);
		if (enabled) { future += now; }
	}
};

struct App
{
	using ModeId = kbd_handler::ModeId;
	using ButtonId = kbd_handler::ButtonId;
	using EventKind = kbd_handler::EventKind;
	using timestamp_type = hrt_abstime;

	ModeId         mode, transition_next_mode;
	bool           transition_requested;
	Tone           tone;
	ButtonId       last_button;

	airleash::DroneCommand drone_cmd;
	airleash::DroneStatus  drone_status;

	PeriodicSayAlive debug_heart_beat;
	Timeout	         timeout;

	void check_drone_status();

	App()
		: mode(ModeId::PREFLIGHT)
		, transition_next_mode(ModeId::NONE)
		, transition_requested(false)
		, last_button(BTN_NONE)
	{}

	void
	set_mode_transition(ModeId m)
	{
		transition_requested = true;
		transition_next_mode = m;
	}

	void
	handle_release()
	{
		using kbd_handler::handle_event;
		if (last_button != BTN_NONE)
		{
			handle_event<EventKind::RELEASE>(*this, mode, last_button);
			last_button = BTN_NONE;
		}
	}

	template <EventKind EVENT>
	void
	handle_press(ButtonId btn)
	{
		static_assert(
			   EVENT == EventKind::SHORT_PRESS
			or EVENT == EventKind::LONG_PRESS
			or EVENT == EventKind::REPEAT_PRESS,
			"handle_press<...> is aplicable onlt to SHORT_PRESS,"
			" LONG_PRESS and REPEAT_PRESS events."
		);

		if (last_button != btn)
		{
			// FIXME Could last button be not NONE and not btn
			// here? Could we remove handle_release() from here?
			handle_release();
		}

		last_button = btn;
		kbd_handler::handle_event<EVENT>(*this, mode, btn);
		tone.key_press();
	}

	void
	handle_time(timestamp_type now)
	{
		using kbd_handler::handle_event;

		debug_heart_beat.check_time(now);
		if (timeout.expired(now)) {
			timeout.disable();
			handle_event<EventKind::TIMEOUT>(*this, mode, BTN_NONE);
			tone.timeout();
		} else {
			drone_status.update();
			if (drone_status.copter_state_has_changed())
				handle_event<EventKind::COPTER_CHANGED_STATE>(*this, mode, BTN_NONE);
		}
	}

	void
	update_state(timestamp_type now)
	{
		// FIXME is NONE required?
		if (transition_requested and transition_next_mode != ModeId::NONE)
		{
			fprintf(stderr, "Mode transition %i -> %i\n",
					mode, transition_next_mode);

			tone.mode_switch();
			mode = transition_next_mode;
			transition_requested = false;
			timeout.restart(now, mode);
		}
	}
};
