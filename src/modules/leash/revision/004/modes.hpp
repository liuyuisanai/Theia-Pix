#pragma once
/*
 * Modes
 */

namespace kbd_handler
{

enum class ModeId : uint8_t
{
	  NONE
	,                LOWER_BOUND // ValueRangeSwitch LOWER_BOUND
	, INIT         = LOWER_BOUND
	, PREFLIGHT
	, MENU
	, CONFIRM_ARM
	, FLIGHT
	, FLIGHT_ALT
	, FLIGHT_CAM
	, SHORTCUT
	, FLIGHT_NO_SIGNAL
	,                UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

constexpr bool
in_air_mode(ModeId m) { return ModeId::FLIGHT <= m; }

constexpr bool
mode_allows_power_off(ModeId m) { return m != ModeId::CONFIRM_ARM; }

} // end of namespace kbd_handler

namespace kbd_handler
{
namespace Debug
{

template <ModeId> struct ModeName;

template <> struct ModeName<ModeId::NONE>
{ static constexpr name_t name = "ModeId::NONE            "; };

template <> struct ModeName<ModeId::INIT>
{ static constexpr name_t name = "ModeId::INIT            "; };

template <> struct ModeName<ModeId::PREFLIGHT>
{ static constexpr name_t name = "ModeId::PREFLIGHT       "; };

template <> struct ModeName<ModeId::MENU>
{ static constexpr name_t name = "ModeId::MENU            "; };

template <> struct ModeName<ModeId::CONFIRM_ARM>
{ static constexpr name_t name = "ModeId::CONFIRM_ARM     "; };

template <> struct ModeName<ModeId::FLIGHT>
{ static constexpr name_t name = "ModeId::FLIGHT          "; };

template <> struct ModeName<ModeId::FLIGHT_ALT>
{ static constexpr name_t name = "ModeId::FLIGHT_ALT      "; };

template <> struct ModeName<ModeId::FLIGHT_CAM>
{ static constexpr name_t name = "ModeId::FLIGHT_CAM      "; };

template <> struct ModeName<ModeId::SHORTCUT>
{ static constexpr name_t name = "ModeId::SHORTCUT        "; };

template <> struct ModeName<ModeId::FLIGHT_NO_SIGNAL>
{ static constexpr name_t name = "ModeId::FLIGHT_NO_SIGNAL"; };

} // end of namespace Debug
} // end of namespace kbd_handler
