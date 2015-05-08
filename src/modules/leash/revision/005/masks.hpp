#pragma once

#define BTN_MASK_POWER  0x001
#define BTN_MASK_PLAY   0x004
#define BTN_MASK_UP     0x008
#define BTN_MASK_DOWN   0x040
#define BTN_MASK_LEFT   0x020
#define BTN_MASK_RIGHT  0x100
#define BTN_MASK_TO_ME  0x200
#define BTN_MASK_TO_H   0x080
#define BTN_MASK_MODE   0x010
#define BTN_MASK_FUTURE 0x002

#define ALL_BUTTONS \
		BTN_MASK_POWER, BTN_MASK_PLAY, \
		BTN_MASK_UP, BTN_MASK_DOWN,    \
		BTN_MASK_LEFT, BTN_MASK_RIGHT, \
		BTN_MASK_TO_H, BTN_MASK_TO_ME, \
		BTN_MASK_MODE, BTN_MASK_FUTURE


namespace kbd_handler
{
namespace Debug
{

template <ButtonId> struct ButtonName;

template <> struct ButtonName<BTN_MASK_POWER>
{ static constexpr name_t name = "BTN_MASK_POWER "; };

template <> struct ButtonName<BTN_MASK_PLAY>
{ static constexpr name_t name = "BTN_MASK_PLAY  "; };

template <> struct ButtonName<BTN_MASK_UP>
{ static constexpr name_t name = "BTN_MASK_UP    "; };

template <> struct ButtonName<BTN_MASK_DOWN>
{ static constexpr name_t name = "BTN_MASK_DOWN  "; };

template <> struct ButtonName<BTN_MASK_LEFT>
{ static constexpr name_t name = "BTN_MASK_LEFT  "; };

template <> struct ButtonName<BTN_MASK_RIGHT>
{ static constexpr name_t name = "BTN_MASK_RIGHT "; };

template <> struct ButtonName<BTN_MASK_TO_ME>
{ static constexpr name_t name = "BTN_MASK_TO_ME "; };

template <> struct ButtonName<BTN_MASK_TO_H>
{ static constexpr name_t name = "BTN_MASK_TO_H  "; };

template <> struct ButtonName<BTN_MASK_MODE>
{ static constexpr name_t name = "BTN_MASK_MODE  "; };

template <> struct ButtonName<BTN_MASK_FUTURE>
{ static constexpr name_t name = "BTN_MASK_FUTURE"; };

} // end of namespace Debug
} // end of namespace kbd_handler
