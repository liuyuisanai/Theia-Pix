#pragma once

#include <cstdint>

#include "bt_types.hpp"
#include "chardev_poll.hpp"
#include "io_recv.hpp"
#include "io_xmit.hpp"
#include "laird/defs.hpp"
#include "mutex.hpp"

namespace BT
{

struct MultiPlexer
{
	struct Flags
	{
		// TODO atomic_channel_mask_t and remove mutex flags.
		channel_mask_t channels_opened_mask;
	} flags;

	RxState rx;
	XtState xt;
	PollMultiPlexer poll_waiters;

	MutexSem mutex_flags;
	MutexSem mutex_rx;
	MutexSem mutex_xt;

	static constexpr auto
	protocol_tag = HostProtocol::LairdProtocol{};
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

ssize_t
read_channel_raw(MultiPlexer & mp, channel_index_t ch, void * buf, size_t buf_size);
ssize_t
write_channel_packet(MultiPlexer & mp, channel_index_t ch, const void * buf, size_t buf_size);

template <typename Device>
void
perform_poll_io(Device & d, MultiPlexer & mp, int poll_timeout_ms);

}
// end of namespace BT
