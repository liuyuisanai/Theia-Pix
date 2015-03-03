#pragma once

#include <cstdint>

#include "bt_types.hpp"
#include "chardev_poll.hpp"
#include "io_recv.hpp"
#include "io_xmit.hpp"
#include "mutex.hpp"

namespace BT
{

struct MultiPlexer
{
	// TODO turn volatile into atomic
	volatile struct Flags
	{
		channel_mask_t channels_opened_mask;

		Flags() : channels_opened_mask(0) {}
	} flags;

	RxState rx;
	XtState xt;
	PollRefSet pollset[8];

	MutexSem mutex_flags;
	MutexSem mutex_rx;
	MutexSem mutex_xt;

	static constexpr LairdParser protocol_tag = LairdParser {};
};

bool
opened_acquare(MultiPlexer & mp, channel_index_t ch);

bool
opened_release(MultiPlexer & mp, channel_index_t ch);

bool
is_channel_xt_ready(const MultiPlexer & mp, channel_index_t ch);

void
set_xt_ready_mask(const MultiPlexer & mp, channel_mask_t mask);

channel_mask_t
get_rx_ready_mask(const MultiPlexer & mp);

}
// end of namespace BT
