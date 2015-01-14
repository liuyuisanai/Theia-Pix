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

struct ShortPress  {};
struct LongPress   {};
struct RepeatPress {};


/*
 * Modes
 */

enum class ModeId
{
	NONE,
	LOWER_BOUND, // ValueRangeSwitch LOWER_BOUND
	A = LOWER_BOUND,
	// B,
	UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

/* previous(ModeId) is required by ValueRangeSwitch implementation */
constexpr ModeId previous(ModeId);

struct ModeA {};

template< ModeId > struct ModeTypeMap { using type = Default; };
template<> struct ModeTypeMap< ModeId::A > { using type = ModeA; };


/*
 * Buttons
 */

using ButtonId = pressed_mask_t;

struct Power  {};
struct Play   {};
struct Up     {};
struct Down   {};
struct Center {};
struct Left   {};
struct Right  {};

template< ButtonId > struct ButtonTypeMap { using type = Default; };
template<> struct ButtonTypeMap< BTN_MASK_POWER  > { using type = Power;  };
template<> struct ButtonTypeMap< BTN_MASK_PLAY   > { using type = Play;   };
template<> struct ButtonTypeMap< BTN_MASK_UP     > { using type = Up;     };
template<> struct ButtonTypeMap< BTN_MASK_DOWN   > { using type = Down;   };
template<> struct ButtonTypeMap< BTN_MASK_CENTER > { using type = Center; };
template<> struct ButtonTypeMap< BTN_MASK_LEFT   > { using type = Left;   };
template<> struct ButtonTypeMap< BTN_MASK_RIGHT  > { using type = Right;  };

} // end of namespace kbd_handler
