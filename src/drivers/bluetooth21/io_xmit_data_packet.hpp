#pragma once

#include <type_traits>

#include "host_protocol.hpp"
#include "buffer_xt.hpp"

namespace BT
{
namespace HostProtocol
{

template <size_t CAPACITY, typename Protocol>
void
append_header(XtBuffer< CAPACITY > & out, DataPacket::DataFrame< Protocol > & frame)
{
	// TODO safety checks
	const auto & header = DataPacket::get_header(frame);
	// TODO optimize insert_end_n_unsafe
	insert_end_unsafe(out, cbegin(header), cend(header));
}

template <size_t CAPACITY, typename Protocol>
typename std::enable_if< DataPacket::has_footer< Protocol >::value >::type
append_footer(XtBuffer< CAPACITY > & out, DataPacket::DataFrame< Protocol > & frame)
{
	// TODO safety checks
	const auto & footer = DataPacket::get_footer(frame);
	// TODO optimize insert_end_n_unsafe
	insert_end_unsafe(out, cbegin(footer), cend(footer));
}

template <size_t CAPACITY, typename Protocol>
typename std::enable_if< not DataPacket::has_footer< Protocol >::value >::type
append_footer(XtBuffer< CAPACITY > & out, DataPacket::DataFrame< Protocol > & frame)
{}

template <typename Protocol, size_t CAPACITY, typename It>
void
append_packet_n_unsafe(channel_index_t ch, XtBuffer< CAPACITY > & out, It buf, size_t buf_size)
{
	auto frame = DataPacket::data_frame< Protocol >(ch, buf_size);
	append_header(out, frame);
	insert_end_n_unsafe(out, buf, buf_size);
	append_footer(out, frame);
};

template <typename Protocol, size_t CAPACITY, typename It>
size_t
transfer_leading_packet(channel_index_t ch, XtBuffer< CAPACITY > & out, It buf, size_t buf_size)
{
	constexpr size_t
	frame_size = DataPacket::frame_size< Protocol >();
	const size_t
	avail = space_available(out);

	if (avail <= frame_size) { return 0; }

	constexpr size_t
	packet_capacity = DataPacket::packet_capacity< Protocol >();
	const size_t
	data_size = min(min(buf_size, packet_capacity), avail - frame_size);

	append_packet_n_unsafe< Protocol >(ch, out, buf, data_size);

	return data_size;
}

template <typename Protocol, size_t CAPACITY, typename It>
size_t
transfer(channel_index_t ch, XtBuffer< CAPACITY > & out, It buf, size_t buf_size)
{
	const size_t s0 = buf_size;
	while (buf_size > 0 and space_available(out) > 0)
	{
		size_t packet_size = transfer_leading_packet< Protocol >(
			ch, out, buf, buf_size
		);
		if (packet_size == 0) { break; }
		buf_size -= packet_size;
	}
	return s0 - buf_size;
}

}
// end of namespace HostProtocol
}
// end of namespace BT
