#pragma once

/*
 * /dev/kbd regularly provides bit-masks where 1 means button was pressed
 * at that moment. Fixed number of last masks are always available.
 *
 * We are interested only in single button events within the following list:
 * + a pressed button was released shortly,
 * + a pressed button was released after some period (like 2 seconds),
 * + a pressed button is being pressed for long period,
 *   then it could be interpreted like repeated presses.
 *
 * Two latter long press variants are mutually exclusive. Interpretation
 * depends on the button and, possibly, mode.
 *
 * The release after long press is out of interest and are not always provided.
 *
 * Masks are compared by equality. Simultaneous presses are not checked, as it
 * is very unlikely to happen unintended with air-leash tight case buttons.
 *
 * When there is heavy CPU load, there could be more than one event in the
 * queue (from the user's point of view). The case is ignored as very unlikely.
 * XXX As a result short press will be skipped
 *     and long press won't get release event.
 */

template <
	typename mask_t, unsigned BUFFER_N_MASKS,
	typename timestamp_t, timestamp_t INTERVAL
>
struct ButtonState
{
	static_assert(INTERVAL > 0, "Scan interval could not be zero.");

	mask_t actual_button;
	timestamp_t time_pressed;
	timestamp_t time_released;
	timestamp_t last_check_time;

	ButtonState()
	: actual_button(0), time_pressed(0), time_released(0), last_check_time(0)
	{}

	void
	init(timestamp_t now) { last_check_time = now; }

	void
	update(timestamp_t now, mask_t masks[BUFFER_N_MASKS])
	{
		// masks[0] is most recent mask, and masks[last] is the oldest one.

		size_t n_masks = (now - last_check_time) / INTERVAL;
		if (n_masks > BUFFER_N_MASKS) { n_masks = BUFFER_N_MASKS; }

		last_check_time = now;

		if (time_released != 0) { actual_button = 0; }

		size_t i = 0;
		while (i < n_masks and masks[i] == actual_button)
			++i;

		if (i == n_masks) { return; } // nothing changed

		if (actual_button == 0 and i == 0)
		{
			// It is a new press.
			actual_button = masks[0];
			// Do not look back for actual press moment,
			// as it does not matter for now.
			time_pressed = now;
			time_released = 0;
		}
		else if (actual_button == 0 /* and i > 0 */)
		{
			// It is a passed by release event.
			// Let it be a short press.
			actual_button = masks[i];
			time_pressed = now - INTERVAL * i;
			time_released = time_pressed;
			// time_released should be (time_pressed + scan
			// interval), but it does not matter for the
			// handling side.
		}
		else if (i == 0)
		{
			if (masks[0] == 0) {
				// It is a release event.
				time_released = now;
			} else {
				// It is a new press.  Ignore release event when
				// there was other button pressed.
				actual_button = masks[i];
				time_pressed = now;
				time_released = 0;
			}
		}
		else //if (i > 0)
		{
			// It is a new press.  Ignore release event when
			// there was other button pressed.
			--i;
			time_pressed = now - INTERVAL * i;
		}
	}
};
