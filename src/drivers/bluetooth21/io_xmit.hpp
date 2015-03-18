#pragma once

#include <cerrno>
#include <cstdio> // perror

#include "std_algo.hpp"
#include "std_iter.hpp"

#include "bt_types.hpp"
#include "buffer_xt.hpp"
#include "debug.hpp"
#include "host_protocol.hpp"
#include "io_xmit_data_packet.hpp"

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

	XtState() : round_robin(0), ready_mask(0xFF) {}
};

inline bool
is_channel_ready(const XtState & xt, channel_index_t i)
{
	return i == 0 or is_set(xt.ready_mask, i);
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
packet_fits_in(XtState & xt, channel_index_t ch, size_t packet_size)
{
	return packet_size <= channel_space_available(xt, ch);
}

// Requirements: ensure packets from different channels are not intermixed.
template <typename Protocol>
inline bool
transfer_raw_channel(Protocol tag, XtState & xt, channel_index_t i)
{
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
	return ready;
}

// Requirements: ensure packets from different channels are not intermixed.
template <typename Protocol>
inline bool
transfer_data_channel(Protocol tag, XtState & xt, channel_index_t i)
{
	auto & out = xt.device_buffer;
	auto & ch = xt.channel_buffer[i];
	bool ready = is_channel_ready(xt, i) and not empty(ch);
	if (ready)
	{
		size_t processed = HostProtocol::transfer< Protocol >(i, out, cbegin(ch), size(ch));
		erase_begin(ch, processed);
		pack(ch);
		ready = processed > 0;
	}
	return ready;
}

template <typename Protocol>
inline poll_notify_mask_t
fill_device_buffer(Protocol tag, XtState & xt)
{
	poll_notify_mask_t poll_mask;

	bool poll_ready = transfer_raw_channel(tag, xt, 0);
	mark(poll_mask, 0, poll_ready);

	// TODO optimize: check available space by minimal packet size.
	if (xt.round_robin == 0 or space_available(xt.device_buffer) > 0)
	{
		auto & i = xt.round_robin;
		auto rr = i;
		do
		{
			if (i != 0)
			{
				poll_ready = transfer_data_channel(tag, xt, i);
				mark(poll_mask, i, poll_ready);
			}
			i = (i + 1) & 7;
		}
		while (i != rr and space_available(xt.device_buffer) > 0);
	}

	return poll_mask;
}

template <typename Protocol, typename Device>
poll_notify_mask_t
process_serial_output(Protocol tag, Device & d, XtState & xt)
{
	poll_notify_mask_t poll_mask = fill_device_buffer(tag, xt);
	ssize_t r = write(d, xt.device_buffer);
	if (r < 0 and errno != EAGAIN)
		perror("process_serial_output / write");
	return poll_mask;
}

inline ssize_t
write_channel_raw(XtState & xt, channel_index_t ch, const void * buf, size_t buf_size)
{
	auto & xt_buf = xt.channel_buffer[ch];

	using char_type = typename XtState::channel_buffer_type::value_type;
	auto data = (const char_type *)buf;

	size_t r = min(space_available(xt_buf), buf_size);

	insert_end_n_unsafe(xt_buf, data, r);

	return r;
}

inline ssize_t
write_channel_packet(XtState & xt, channel_index_t ch, const void * buf, size_t buf_size)
{
	// FIXME Should we try to pack() or clear()?
	if (packet_fits_in(xt, ch, buf_size))
	{
		using char_type = typename XtState::channel_buffer_type::value_type;
		auto data = (const char_type *)buf;

		insert_end_n_unsafe(xt.channel_buffer[ch], data, buf_size);
		return buf_size;
	}

	return 0;
}

inline void
drain(XtState & xt, channel_index_t ch) { clear(xt.channel_buffer[ch]); }

inline void
dbg_dump(const char comment[], XtState & xt)
{
	if (size(xt.channel_buffer[0])
	    + size(xt.channel_buffer[1])
	    + size(xt.channel_buffer[2])
	    + size(xt.channel_buffer[3])
	    + size(xt.channel_buffer[4])
	    + size(xt.channel_buffer[5])
	    + size(xt.channel_buffer[6])
	    + size(xt.channel_buffer[7])
	    + size(xt.device_buffer)
	    == 0
	) { return; }

	dbg("%s: Xt channels %u %u %u %u %u %u %u %u uart %u\n"
		, comment
		, space_available(xt.channel_buffer[0])
		, space_available(xt.channel_buffer[1])
		, space_available(xt.channel_buffer[2])
		, space_available(xt.channel_buffer[3])
		, space_available(xt.channel_buffer[4])
		, space_available(xt.channel_buffer[5])
		, space_available(xt.channel_buffer[6])
		, space_available(xt.channel_buffer[7])
		, space_available(xt.device_buffer)
	);
}

}
// end of namespace BT
