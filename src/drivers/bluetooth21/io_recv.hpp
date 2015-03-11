#pragma once

#include <cerrno>
#include <cstdio> // perror

#include "bt_types.hpp"
#include "buffer_rx.hpp"
#include "debug.hpp"
#include "host_protocol.hpp"

#include "std_algo.hpp"
#include "std_iter.hpp"
#include "std_util.hpp"


namespace BT
{

struct RxState
{
	using device_buffer_type = RxBuffer< 256 >;
	using channel_buffer_type = RxBuffer< 256 >;

	device_buffer_type device_buffer;
	channel_buffer_type channel_buffer[8];
};

template <typename Protocol, typename Device>
channel_mask_t
process_serial_input(Protocol tag, Device & d, RxState & rx)
{
	using namespace HostProtocol::Parser;
	using DevIt = typename RxState::device_buffer_type::const_iterator;

	channel_mask_t poll_mask;

	ssize_t r = read(d, rx.device_buffer);
	if (r < 0 and errno != EAGAIN)
		perror("process_serial_input / read");

	while (not empty(rx.device_buffer))
	{
		DevIt first, last;
		tie(first, last) = HostProtocol::Parser::find_next_packet(
			tag,
			cbegin(rx.device_buffer),
			cend(rx.device_buffer)
		);

		// TODO Drop bad prefix fragment.
		// rx.device_buffer.erase(begin(rx.device_buffer), first);
		// if (rx.device_buffer.empty())
		// 	break;

		if (first == last)
			/* there is no whole packet */
			break;

		channel_index_t ch = get_channel_number(tag, first, last);

		// TODO check channel index is valid.
		// TODO check channel is opened. Drop data on closed channels.

		mark(poll_mask, ch, true);

		auto & ch_buf = rx.channel_buffer[ch];
		DevIt data_first, data_last;
		tie(data_first, data_last) =
			get_packet_data_slice(tag, first, last);

		size_t data_size = distance(data_first, data_last);
		if (space_available(ch_buf) < data_size) { clear(ch_buf); }
		else { pack(ch_buf); }

		insert_end_unsafe(ch_buf, data_first, data_last);
		erase_begin(
			rx.device_buffer,
			distance(cbegin(rx.device_buffer), last)
		);
	}
	pack(rx.device_buffer);

	return poll_mask;
}

inline ssize_t
read_channel_raw(RxState & rx, channel_index_t ch, void * buf, size_t buf_size)
{
	auto & rx_buf = rx.channel_buffer[ch];

	auto h = min<size_t>(size(rx_buf), buf_size);

	using char_type = typename RxState::channel_buffer_type::value_type;
	auto b = (char_type *)buf;

	copy_n(cbegin(rx_buf), h, b);
	erase_begin(rx_buf, h);

	return h;
}

//inline bool
//service_channel_empty(RxState & rx)
//{
//	return empty(rx.channel_buffer[0]);
//}
//
//template <typename Protocol>
//ssize_t
//read_service_channel(Protocol tag, RxState & rx, void * buf, size_t buf_size)
//{
//	auto & rx_buf = rx.channel_buffer[0];
//
//	using BufIt = typename RxState::channel_buffer_type::const_iterator;
//	BufIt first, last;
//	tie(first, last) =
//		find_next_packet(tag, cbegin(rx_buf), cend(rx_buf));
//
//	if (first == last)
//	{
//		errno = EAGAIN;
//		return -1;
//	}
//
//	// TODO
//	//if (first != begin(rx_buf) {}
//
//	size_t s = distance(first, last);
//	if (buf_size < s)
//	{
//		errno = ENOMEM;
//		return -1;
//	}
//
//	using char_type = typename RxState::channel_buffer_type::value_type;
//	auto b = (char_type *)buf;
//
//	copy(first, last, b);
//	erase_begin(rx_buf, s);
//
//	return s;
//}
//
inline void
drain(RxState & rx, channel_index_t ch) { clear(rx.channel_buffer[ch]); }

inline void
dbg_dump(const char comment[], RxState & rx)
{
	dbg("%s: Rx channels %u %u %u %u %u %u %u %u uart %u\n"
		, comment
		, size(rx.channel_buffer[0])
		, size(rx.channel_buffer[1])
		, size(rx.channel_buffer[2])
		, size(rx.channel_buffer[3])
		, size(rx.channel_buffer[4])
		, size(rx.channel_buffer[5])
		, size(rx.channel_buffer[6])
		, size(rx.channel_buffer[7])
		, size(rx.device_buffer)
	);
}

} // end of namespace BT
