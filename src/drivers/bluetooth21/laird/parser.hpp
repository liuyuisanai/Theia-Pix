#pragma once

#include "../std_algo.hpp"
#include "../std_iter.hpp"
#include "../std_util.hpp"

#include "defs.hpp"

namespace BT
{
namespace HostProtocol
{
namespace Parser
{

template <typename Iterator>
std::pair<Iterator, Iterator>
find_next_packet(LairdProtocol, Iterator first, Iterator last)
{
	// TODO skip leading zeros, ones, twos.
	size_t n_bytes = distance(first, last);
	size_t packet_size = (n_bytes > 2 and *first <= n_bytes) ? *first : 0;
	last = packet_size <= n_bytes ? next(first, packet_size) : first;
	return std::make_pair(first, last);
}

template <typename Iterator>
channel_index_t
get_channel_number(LairdProtocol, Iterator first, Iterator last)
{
	++first;
	return *first;
}

template <typename Iterator>
std::pair<Iterator, Iterator>
get_packet_data_slice(LairdProtocol tag, Iterator first, Iterator last)
{
	channel_index_t ch = get_channel_number(tag, first, last);
	if (ch == 0)
		/* service channel */
		return std::make_pair(first, last);
	/* data channel */
	return std::make_pair(next(first, 2), last);
}

}
// end of namespace Parser
}
// end of namespace HostProtocol
}
// end of namespace BT
