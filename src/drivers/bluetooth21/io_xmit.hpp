#pragma once

#include <cerrno>
#include <cstdio>

#include "bt_types.hpp"
#include "buffer_xt.hpp"
#include "laird_parser.hpp"

#include "std_algo.hpp"
#include "std_iter.hpp"


namespace BT
{

struct XtState
{
	using device_buffer_type = XtBuffer< 256 >;
	using channel_buffer_type = XtBuffer< 256 >;

	device_buffer_type device_buffer;
	channel_buffer_type channel_buffer[8];

	channel_index_t round_robin;
	channel_mask_t ready_mask;

	XtState() : round_robin(0), ready_mask(0) {}
};

inline bool
is_channel_ready(const XtState & xt, channel_index_t i)
{
	return i == 0 or xt.ready_mask & (1 << i);
}

inline size_t
channel_space_available(XtState & xt, channel_index_t i)
{
	return space_available(xt.channel_buffer[i]);
}

inline bool
xt_is_waiting(XtState & xt)
{
	return not empty(xt.device_buffer);
}

inline bool
command_is_waiting(XtState & xt)
{
	return not empty(xt.channel_buffer[0]);
}

inline bool
packet_fits_in(XtState & xt, size_t cmd_size)
{
	return cmd_size <= channel_space_available(xt, 0);
}

// Requirements: ensure packets from different channels are not intermixed.
inline void
transfer_channel_data(XtState & xt, channel_index_t i)
{
	// TODO Do packet-wise transfer, allowing older packets sent earlier.

	auto & out = xt.device_buffer;
	auto & ch = xt.channel_buffer[i];
	bool ready = is_channel_ready(xt, i)
		and not empty(ch)
		and size(ch) <= space_available(out);
	if (ready)
	{
		insert_end_unsafe(out, cbegin(ch), cend(ch));
		clear(ch);
	}
}

inline void
fill_device_buffer(XtState & xt)
{
	transfer_channel_data(xt, 0);

	// TODO optimize: check available space by minimal packet size.
	if (space_available(xt.device_buffer) == 0)
		return;

	auto & i = xt.round_robin;
	auto rr = i;
	do
	{
		transfer_channel_data(xt, i);
		i = (i + 1) & 7;
	}
	while (i != rr and space_available(xt.device_buffer) > 0);
}

template <typename Device>
void
process_serial_output(Device & d, XtState & xt)
{
	fill_device_buffer(xt);
	ssize_t r = write(d, xt.device_buffer);
	if (r < 0 and errno != EAGAIN)
		perror("process_serial_output / write");
}

inline ssize_t
write_channel_raw(XtState & xt, channel_index_t ch, const void * buf, size_t buf_size)
{
	auto & xt_buf = xt.channel_buffer[ch];

	using char_type = typename XtState::channel_buffer_type::value_type;
	auto data = (const char_type *)buf;

	size_t r = min(space_available(xt_buf), buf_size);

	insert_end_unsafe(xt_buf, data, data + r);

	return r;
}

inline ssize_t
write_channel_packet(XtState & xt, channel_index_t ch, const void * buf, size_t buf_size)
{
	if (packet_fits_in(xt, buf_size))
		return write_channel_raw(xt, ch, buf, buf_size);

	return 0;
}

inline void
drain(XtState & xt, channel_index_t ch) { clear(xt.channel_buffer[ch]); }

}
// end of namespace BT
