#pragma once

namespace kbd_handler
{

template <TimeoutKind>
static bool
define_timeout(ModeId m, hrt_abstime & duration);

template <>
static bool
define_timeout<TimeoutKind::COPTER_STATE_REFRESH>(ModeId m, hrt_abstime & duration)
{
	switch (m)
	{
	// State refresh timeouts
	case ModeId::INIT:
	case ModeId::PREFLIGHT:
	case ModeId::FLIGHT:
	case ModeId::FLIGHT_NO_SIGNAL:
		duration =   500000u; /*  0.5s */
		break;
	// No timeout
	default:
		return false;
	}
	return true;
}

template <>
static bool
define_timeout<TimeoutKind::KEY_PRESS>(ModeId m, hrt_abstime & duration)
{
	switch (m)
	{
	// Fast timeouts
	case ModeId::CONFIRM_ARM:
		duration =  5000000u; /*  5.0s */
		break;
	// Slow timeouts
	case ModeId::FLIGHT_ALT:
    case ModeId::FLIGHT_CAM:
	case ModeId::SHORTCUT:
		duration = 10000000u; /* 10.0s */
		break;
	// No timeout
	default:
		return false;
	}
	return true;
}

} // end of namespace kbd_handler
