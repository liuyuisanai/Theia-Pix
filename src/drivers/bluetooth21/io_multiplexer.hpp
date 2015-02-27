#pragma once

#include <cstdint>

#include "bt_types.hpp"
#include "mutex.hpp"

namespace BT
{

struct MultiPlexer
{
	volatile struct Flags
	{
		uint8_t channels_opened_mask;

		Flags() : channels_opened_mask(0) {}
	} flags;

	MutexSem mutex_flags;
	MutexSem mutex_rx;
	MutexSem mutex_xt;
};

bool
opened_acquare(MultiPlexer & mp, channel_index_t ch);
bool
opened_release(MultiPlexer & mp, channel_index_t ch);

}
// end of namespace BT
