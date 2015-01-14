#pragma once

#define LONG_PRESS_DURATION_us 1500000u

#define BTN_MASK_POWER  0x001
#define BTN_MASK_PLAY   0x004
#define BTN_MASK_UP     0x008
#define BTN_MASK_DOWN   0x040
#define BTN_MASK_CENTER 0x200
#define BTN_MASK_LEFT   0x020
#define BTN_MASK_RIGHT  0x100

#define ALL_BUTTONS \
		BTN_MASK_POWER, BTN_MASK_PLAY, \
		BTN_MASK_UP, BTN_MASK_DOWN, \
		BTN_MASK_CENTER, \
		BTN_MASK_LEFT, BTN_MASK_RIGHT


namespace kbd_handler
{

struct Default {};

/*
 * Events
 */

enum EventKind
{
	SHORT_PRESS,
	LONG_PRESS,
	REPEAT_PRESS
};


/*
 * Modes
 */

enum class ModeId
{
	NONE,
	LOWER_BOUND, // ValueRangeSwitch LOWER_BOUND
	A = LOWER_BOUND,
	DRIFT,
	UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

/* previous(ModeId) is required by ValueRangeSwitch implementation */
constexpr ModeId previous(ModeId);


/*
 * Buttons
 */

using ButtonId = pressed_mask_t;

} // end of namespace kbd_handler
