#pragma once

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

template <> struct ButtonName<BTN_MASK_CENTER>
{ static constexpr name_t name = "BTN_MASK_CENTER"; };

template <> struct ButtonName<BTN_MASK_LEFT>
{ static constexpr name_t name = "BTN_MASK_LEFT  "; };

template <> struct ButtonName<BTN_MASK_RIGHT>
{ static constexpr name_t name = "BTN_MASK_RIGHT "; };

} // end of namespace Debug
} // end of namespace kbd_handler
