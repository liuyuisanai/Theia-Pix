#pragma once

#include <cstdint>

#include "std_array.hpp"

namespace BT
{

using channel_index_t = uint8_t;
struct channel_mask_t;

using Address6 = PODArray<uint8_t, 6>;
#define Address6_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define Address6_FMT_ITEMS(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5]


struct channel_mask_t
{
	uint8_t value;

	channel_mask_t(uint8_t x=0) : value(x) {}
	channel_mask_t(const channel_mask_t &) = default;

	channel_mask_t &
	operator = (const channel_mask_t &) = default;

	channel_mask_t &
	operator |= (const channel_mask_t & other)
	{
		value |= other.value;
		return *this;
	}
};

using poll_notify_mask_t = channel_mask_t;


inline bool
empty(const channel_mask_t & mask) { return mask.value == 0; }

inline bool
is_set(const channel_mask_t & mask, channel_index_t i)
{
	return mask.value & (1 << i);
}

inline void
mark(channel_mask_t & mask, channel_index_t i, bool on)
{
	uint8_t bit = 1 << i;
	if (on) { mask.value |= bit; }
	else { mask.value &= ~bit; }
}

} // end of namespace BT
