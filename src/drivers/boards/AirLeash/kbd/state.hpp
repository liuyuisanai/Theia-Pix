#pragma once

#include <drivers/drv_airleash_kbd.h>

#include "stm32_helper.hpp"

namespace airleash_kbd {

struct ScanState {
	pressed_mask_t pressed_keys_mask, next_mask;

	ScanState() : pressed_keys_mask{0}, next_mask{0} {}
};

inline void
start_next_cycle(ScanState & s)
{
	s.pressed_keys_mask = s.next_mask;
	s.next_mask = 0;
}

inline void
add_bool(ScanState & s, bool on)
{
	s.next_mask <<= 1;
	if (on) { s.next_mask |= 1; }
}

inline void
add_bit(ScanState & s, pressed_mask_t bit)
{
	s.next_mask <<= 1;
	s.next_mask |= bit;
}

template <typename MASK_T>
inline void
add_bits_by_masks(ScanState & s, MASK_T x)
{}

template <typename MASK_T, typename T1, typename ... T>
inline void
add_bits_by_masks(ScanState & s, MASK_T x, T1 bit_mask, T... tail)
{
	add_bool(s, x & bit_mask);
	add_bits_by_masks(s, x, tail...);
}

inline void
add_gpio_pins(ScanState &, reg_value_t)
{}

template <typename T1, typename ... T>
inline void
add_gpio_pins(ScanState & s, reg_value_t reg, T1 pin_index, T... tail)
{
	add_bit(s, (reg >> gpio_reg_pin_shift(pin_index)) & 1);
	add_gpio_pins(s, reg, tail...);
}

} // end of namespace airleash_kbd
