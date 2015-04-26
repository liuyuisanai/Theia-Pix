#pragma once

#include <cstdint>

#include "std_array.hpp"

namespace BT
{

using Address6 = PODArray<uint8_t, 6>;
#define Address6_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define Address6_FMT_ITEMS(x) (x)[0], (x)[1], (x)[2], (x)[3], (x)[4], (x)[5]

using LinkKey16 = PODArray<uint8_t, 16>;

using channel_index_t = uint8_t; // 0..7
using connection_index_t = uint8_t;  // 1..7
using device_index_t = uint8_t;  // 0..7
constexpr connection_index_t INVALID_DEV_INDEX = 255;

struct bitmask_t
{
	uint8_t value;

	bitmask_t(uint8_t x=0) : value(x) {}
	bitmask_t(const bitmask_t &) = default;

	bitmask_t &
	operator = (const bitmask_t &) = default;

	inline bitmask_t &
	operator |= (const bitmask_t & other)
	{
		value |= other.value;
		return *this;
	}

	friend inline bool
	operator == (const bitmask_t & a, const bitmask_t & b)
	{ return a.value == b.value; }

	friend inline bool
	operator != (const bitmask_t & a, const bitmask_t & b)
	{ return not (a == b); }
};

using channel_mask_t = bitmask_t;
using device_index_mask_t = bitmask_t;
using poll_notify_mask_t = bitmask_t;


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

inline channel_mask_t
operator - (const channel_mask_t & a, const channel_mask_t & b)
{ return channel_mask_t(a.value & ~b.value); }

} // end of namespace BT
