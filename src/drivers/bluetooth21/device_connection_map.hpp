#pragma once

#include "bt_types.hpp"

namespace BT
{

struct MultiPlexer;

struct DeviceConnectionMap
{
	channel_mask_t connected_mask;
	connection_index_t map_channel_conn[7];
	channel_index_t map_conn_channel[7];

	DeviceConnectionMap()
	: map_conn_channel{0, 0, 0, 0, 0, 0, 0}
	{}
};

inline bool
is_connection_established(const DeviceConnectionMap & self, connection_index_t n)
{ return self.map_conn_channel[n - 1] != 0; }

inline channel_index_t
channel_index(DeviceConnectionMap & self, connection_index_t n)
{ return self.map_conn_channel[n - 1]; }

void
update_by_mask(DeviceConnectionMap & self, channel_mask_t connected, MultiPlexer & mp);

}
// end of namespace BT
