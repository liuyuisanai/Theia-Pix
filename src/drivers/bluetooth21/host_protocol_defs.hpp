#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility> // std::pair

#include "bt_types.hpp"

namespace BT
{
namespace HostProtocol
{

namespace Parser
{

template <typename Protocol, typename Iterator>
std::pair<Iterator, Iterator>
find_next_packet(Protocol, Iterator first, Iterator last);

template <typename Protocol, typename Iterator>
channel_index_t
get_channel_number(Protocol, Iterator first, Iterator last);

template <typename Protocol, typename Iterator>
std::pair<Iterator, Iterator>
get_packet_data_slice(Protocol, Iterator first, Iterator last);

}
// end of namespace Parser

namespace DataPacket
{

template <typename Protocol>
constexpr size_t
frame_size();

template <typename Protocol>
constexpr size_t
packet_capacity();

template <typename Protocol>
struct DataFrame;

template <typename Protocol>
constexpr DataFrame< Protocol >
data_frame(channel_index_t ch, size_t data_size)
{ return DataFrame< Protocol >(ch, data_size ); }

template <typename Protocol>
typename DataFrame< Protocol >::header_type constexpr
get_header(const DataFrame< Protocol > &);

template <typename Protocol>
typename DataFrame< Protocol >::footer_type constexpr
get_footer(const DataFrame< Protocol > &);

// TODO try no_footer = is_void
template <typename Protocol>
struct has_footer
{
	static constexpr bool
	value = not std::is_void<
		typename DataFrame< Protocol >::footer_type
	>::value;
};

}
// end of namespace DataPacket

}
// end of namespace HostProtocol
}
// end of namespace BT
