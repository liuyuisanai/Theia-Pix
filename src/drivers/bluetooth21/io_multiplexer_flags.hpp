#pragma once

#include <cstdint>

#include "debug.hpp"
#include "io_multiplexer.hpp"
#include "std_algo.hpp"

namespace BT
{

/* Flag operations */

inline bool
opened_acquare(MultiPlexer & mp, channel_index_t ch)
{
	lock_guard guard(mp.mutex_flags);

	uint8_t channel_bit = 1 << ch;
	uint8_t old_mask = mp.flags.channels_opened_mask.value;
	uint8_t new_mask = old_mask | channel_bit;
	mp.flags.channels_opened_mask.value = new_mask;

	return not (old_mask & channel_bit);
}

inline bool
opened_release(MultiPlexer & mp, channel_index_t ch)
{
	lock_guard guard(mp.mutex_flags);

	uint8_t channel_bit = 1 << ch;
	uint8_t old_mask = mp.flags.channels_opened_mask.value;
	uint8_t new_mask = old_mask & ~channel_bit;
	mp.flags.channels_opened_mask.value = new_mask;

	return old_mask & channel_bit;
}

inline bool
is_channel_xt_ready(const MultiPlexer & mp, channel_index_t ch)
{ return is_channel_ready(mp.xt, ch); }

inline void
set_xt_ready_mask(MultiPlexer & mp, channel_mask_t mask)
{ mp.xt.ready_mask.value = mask.value; }

inline channel_mask_t
get_rx_ready_mask(const MultiPlexer & mp)
{
	// We assume buffers are big enough for mavlink's small data volume.
	// So any opened channel can receive data.
	return mp.flags.channels_opened_mask;
}

}
// end of namespace BT
