#pragma once

#include <drivers/drv_airleash_kbd.h>

#define BTN_NONE        0

namespace kbd_handler
{

struct Default {};

namespace Debug { using name_t = const char * const; }


/*
 * Events
 */

enum EventKind : uint8_t
{
	  SHORT_KEYPRESS
	, LONG_KEYPRESS
	, REPEAT_KEYPRESS
	, KEY_RELEASE
	, KEY_TIMEOUT
	, COPTER_CHANGED_STATE
};

constexpr bool
event_is_short_or_repeat_press(EventKind E)
{ return E == EventKind::SHORT_KEYPRESS or E == EventKind::REPEAT_KEYPRESS; }

namespace Debug
{

template <EventKind> struct EventName;

template <> struct EventName<EventKind::SHORT_KEYPRESS       >
{ static constexpr name_t name = "EventKind::SHORT_KEYPRESS      "; };

template <> struct EventName<EventKind::LONG_KEYPRESS        >
{ static constexpr name_t name = "EventKind::LONG_KEYPRESS       "; };

template <> struct EventName<EventKind::REPEAT_KEYPRESS      >
{ static constexpr name_t name = "EventKind::REPEAT_KEYPRESS     "; };

template <> struct EventName<EventKind::KEY_RELEASE          >
{ static constexpr name_t name = "EventKind::KEY_RELEASE         "; };

template <> struct EventName<EventKind::KEY_TIMEOUT          >
{ static constexpr name_t name = "EventKind::KEY_TIMEOUT         "; };

template <> struct EventName<EventKind::COPTER_CHANGED_STATE >
{ static constexpr name_t name = "EventKind::COPTER_CHANGED_STATE"; };

} // end of namespace Debug


/*
 * Modes
 */

enum class ModeId : uint8_t;

constexpr bool
in_air_mode(ModeId);

constexpr bool
mode_allows_power_off(ModeId);

/* previous(ModeId) is required by ValueRangeSwitch implementation */
constexpr ModeId
previous(ModeId);

namespace Debug { template <ModeId> struct ModeName; }


/*
 * Buttons
 */

using ButtonId = pressed_mask_t;

namespace Debug
{

template <ButtonId> struct ButtonName
{ static constexpr name_t name = "ButtonId(-?-)  "; };

template <> struct ButtonName<BTN_NONE>
{ static constexpr name_t name = "BTN_NONE       "; };

} // end of namespace Debug


/*
 * Prototypes
 */

template <EventKind, typename State>
void
handle_event(State &, ModeId m, ButtonId b);

bool
has_repeated_press(ModeId m, ButtonId b);

} // end of namespace kbd_handler

#ifndef CONFIG_BOARD_REVISION
# error Define non-empty CONFIG_BOARD_REVISION.
#endif
#if CONFIG_BOARD_REVISION <= 4
# include "revision/004/masks.hpp"
# include "revision/004/modes.hpp"
#else
# include "revision/005/masks.hpp"
# include "revision/005/modes.hpp"
#endif
