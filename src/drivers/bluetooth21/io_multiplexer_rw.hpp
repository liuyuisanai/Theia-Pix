#pragma once

#include "io_multiplexer.hpp"

namespace BT
{

ssize_t
read_channel_raw(MultiPlexer & mp, channel_index_t ch, void * buf, size_t buf_size)
{
	lock_guard_interruptable guard(mp.mutex_rx);
	if (not guard.locked) { return -EINTR; }
	return read_channel_raw(mp.rx, ch, buf, buf_size);
}

ssize_t
read_service_channel(MultiPlexer & mp, void * buf, size_t buf_size)
{
	lock_guard_interruptable guard(mp.mutex_rx);
	if (not guard.locked) { return -EINTR; }
	return read_service_channel(mp.protocol_tag, mp.rx, buf, buf_size);
}

ssize_t
write_channel_packet(MultiPlexer & mp, channel_index_t ch, const void * buf, size_t buf_size)
{
	lock_guard_interruptable guard(mp.mutex_xt);
	if (not guard.locked) { return -EINTR; }
	return write_channel_packet(mp.xt, ch, buf, buf_size);
}

}
// end of namespace BT
