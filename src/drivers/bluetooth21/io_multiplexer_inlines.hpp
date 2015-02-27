#pragma once

#include <cstdint>

#include "debug.hpp"
#include "io_multiplexer.hpp"

namespace BT
{

/* Flag operations */

inline bool
opened_acquare(MultiPlexer & mp, channel_index_t ch)
{
	lock_guard guard(mp.mutex_flags);

	uint8_t channel_bit = 1 << ch;
	uint8_t old_mask = mp.flags.channels_opened_mask;
	uint8_t new_mask = old_mask | channel_bit;
	mp.flags.channels_opened_mask = new_mask;

	return not (old_mask & channel_bit);
}

inline bool
opened_release(MultiPlexer & mp, channel_index_t ch)
{
	lock_guard guard(mp.mutex_flags);

	uint8_t channel_bit = 1 << ch;
	uint8_t old_mask = mp.flags.channels_opened_mask;
	uint8_t new_mask = old_mask & ~channel_bit;
	mp.flags.channels_opened_mask = new_mask;

	return old_mask & channel_bit;
}

}
// end of namespace BT
