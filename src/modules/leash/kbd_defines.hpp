#pragma once

#define LONG_PRESS_DURATION_us 1500000u

#define BTN_NONE        0
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
	  SHORT_PRESS
	, LONG_PRESS
	, REPEAT_PRESS
	, RELEASE
	, TIMEOUT
	, COPTER_CHANGED_STATE
};

using name_t = const char * const;
template <EventKind> struct EventDebugName;
template <> struct EventDebugName<EventKind::SHORT_PRESS          > { static constexpr name_t name = "EventKind::SHORT_PRESS         "; };
template <> struct EventDebugName<EventKind::LONG_PRESS           > { static constexpr name_t name = "EventKind::LONG_PRESS          "; };
template <> struct EventDebugName<EventKind::REPEAT_PRESS         > { static constexpr name_t name = "EventKind::REPEAT_PRESS        "; };
template <> struct EventDebugName<EventKind::RELEASE              > { static constexpr name_t name = "EventKind::RELEASE             "; };
template <> struct EventDebugName<EventKind::TIMEOUT              > { static constexpr name_t name = "EventKind::TIMEOUT             "; };
template <> struct EventDebugName<EventKind::COPTER_CHANGED_STATE > { static constexpr name_t name = "EventKind::COPTER_CHANGED_STATE"; };


/*
 * Modes
 */

enum class ModeId
{
	NONE,
	               LOWER_BOUND, // ValueRangeSwitch LOWER_BOUND
	PREFLIGHT    = LOWER_BOUND,
	MENU,
	CONFIRM_ARM,
	FLIGHT,
	FLIGHT_ALT,
	SHORTCUT,
	               UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

template <ModeId> struct ModeDebugName;
template <> struct ModeDebugName<ModeId::NONE       > { static constexpr name_t name = "ModeId::NONE       "; };
template <> struct ModeDebugName<ModeId::PREFLIGHT  > { static constexpr name_t name = "ModeId::PREFLIGHT  "; };
template <> struct ModeDebugName<ModeId::FLIGHT     > { static constexpr name_t name = "ModeId::FLIGHT     "; };
template <> struct ModeDebugName<ModeId::FLIGHT_ALT > { static constexpr name_t name = "ModeId::FLIGHT_ALT "; };
template <> struct ModeDebugName<ModeId::SHORTCUT   > { static constexpr name_t name = "ModeId::SHORTCUT   "; };
template <> struct ModeDebugName<ModeId::MENU       > { static constexpr name_t name = "ModeId::MENU       "; };
template <> struct ModeDebugName<ModeId::CONFIRM_ARM> { static constexpr name_t name = "ModeId::CONFIRM_ARM"; };

/* previous(ModeId) is required by ValueRangeSwitch implementation */
constexpr ModeId previous(ModeId);


/*
 * Buttons
 */

using ButtonId = pressed_mask_t;

template <ButtonId> struct ButtonDebugName { static constexpr name_t name = "ButtonId(unknown)"; };
template <> struct ButtonDebugName<BTN_NONE       > { static constexpr name_t name = "BTN_NONE       "; };
template <> struct ButtonDebugName<BTN_MASK_POWER > { static constexpr name_t name = "BTN_MASK_POWER "; };
template <> struct ButtonDebugName<BTN_MASK_PLAY  > { static constexpr name_t name = "BTN_MASK_PLAY  "; };
template <> struct ButtonDebugName<BTN_MASK_UP    > { static constexpr name_t name = "BTN_MASK_UP    "; };
template <> struct ButtonDebugName<BTN_MASK_DOWN  > { static constexpr name_t name = "BTN_MASK_DOWN  "; };
template <> struct ButtonDebugName<BTN_MASK_CENTER> { static constexpr name_t name = "BTN_MASK_CENTER"; };
template <> struct ButtonDebugName<BTN_MASK_LEFT  > { static constexpr name_t name = "BTN_MASK_LEFT  "; };
template <> struct ButtonDebugName<BTN_MASK_RIGHT > { static constexpr name_t name = "BTN_MASK_RIGHT "; };


template <EventKind, typename State>
void
handle_event(State &, ModeId m, ButtonId b);

} // end of namespace kbd_handler
