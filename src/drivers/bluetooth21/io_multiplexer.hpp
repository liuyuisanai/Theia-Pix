#pragma once

#include <cstdint>

#include "bt_types.hpp"
#include "chardev_poll.hpp"
#include "device_connection_map.hpp"
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
	DeviceConnectionMap connection_slots;

	MutexSem mutex_flags;
	MutexSem mutex_rx;
	MutexSem mutex_xt;

	static constexpr auto
	protocol_tag = HostProtocol::LairdProtocol{};
};

bool
opened_acquare(MultiPlexer & mp, device_index_t di);

bool
opened_release(MultiPlexer & mp, device_index_t di);

bool
is_channel_xt_ready(const MultiPlexer & mp, channel_index_t ch);

void
set_xt_ready_mask(const MultiPlexer & mp, channel_mask_t mask);

ssize_t
read_channel_raw(MultiPlexer & mp, channel_index_t ch, void * buf, size_t buf_size);
ssize_t
write_channel_packet(MultiPlexer & mp, channel_index_t ch, const void * buf, size_t buf_size);

template <typename Device>
void
perform_poll_io(Device & d, MultiPlexer & mp, int poll_timeout_ms);

void
update_connections(MultiPlexer & mp, channel_mask_t connected);

inline bool
is_healthy(const MultiPlexer & mp) { return mp.rx.healthy; }

}
// end of namespace BT
