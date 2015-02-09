#pragma once
/*
 * Modes
 */

namespace kbd_handler
{

enum class ModeId : uint8_t
{
	NONE,
	               LOWER_BOUND, // ValueRangeSwitch LOWER_BOUND
	INIT         = LOWER_BOUND,
	PREFLIGHT,
	MENU,
	CONFIRM_ARM,
	FLIGHT,
	FLIGHT_ALT,
    FLIGHT_CAM,
	SHORTCUT,
	FLIGHT_NO_SIGNAL,
	               UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

} // end of namespace kbd_handler
