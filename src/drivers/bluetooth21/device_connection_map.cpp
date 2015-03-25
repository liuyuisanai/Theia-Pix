#include "debug.hpp"
#include "device_connection_map.hpp"
#include "io_multiplexer.hpp"
#include "io_multiplexer_flags.hpp"

namespace BT
{

inline void
on_connection_closed(MultiPlexer & mp, channel_index_t ch)
{
	// FIXME drain(mp.rx, ch);
	// FIXME drain(mp.xt, ch);
	clear(mp.poll_waiters[ch]);
}

inline void
on_connection_established(MultiPlexer & mp, channel_index_t ch)
{
	// FIXME drain(mp.rx, ch);
	// FIXME drain(mp.xt, ch);
}

inline connection_index_t &
connection_index(DeviceConnectionMap & self, channel_index_t ch)
{ return self.map_channel_conn[ch - 1]; }

inline connection_index_t
first_free_conn(const DeviceConnectionMap & self)
{ return find_n2(self.map_conn_channel, 7, 0).second; }

void
update_by_mask(DeviceConnectionMap & self, channel_mask_t connected, MultiPlexer & mp)
{
	for (channel_index_t ch = 1; ch < 8; ++ch)
	{
		const bool up = is_set(connected, ch);
		if (up != is_set(self.connected_mask, ch))
		{
			dbg("DeviceConnectionMap ch %u -> %s.\n"
				, ch
				, up ? "established" : "dropped"
			);
			if (up)
			{
				const auto ci = first_free_conn(self);
				/*
				 * free slot is always available here.
				 */
				connection_index(self, ch) = ci;
				self.map_conn_channel[ci] = ch;
				on_connection_established(mp, ch);
				dbg("Channel %u mapped to connection %u.\n", ch, ci);
			}
			else
			{
				const auto ci = connection_index(self, ch);
				self.map_conn_channel[ci] = 0;
				on_connection_closed(mp, ci);
				dbg("Closed channel %u, connection %u.\n", ch, ci);
			}
		}
	}
	self.connected_mask = connected;
}

}
// end of namespace BT
