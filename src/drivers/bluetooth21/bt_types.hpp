#pragma once

#include <cstdint>

namespace BT
{

using channel_index_t = uint8_t;

struct channel_mask_t
{
	uint8_t value;

	channel_mask_t() : value(0) {}
	channel_mask_t(const channel_mask_t &) = default;

	channel_mask_t&
	operator = (const channel_mask_t &) = default;
};

inline void
mark(channel_mask_t & mask, channel_index_t i, bool on)
{
	uint8_t bit = 1 << i;
	if (on) { mask.value |= bit; }
	else { mask.value &= ~bit; }
}

inline bool
is_set(const channel_mask_t & mask, channel_index_t i)
{
	return mask.value & (1 << i);
}

} // end of namespace BT
