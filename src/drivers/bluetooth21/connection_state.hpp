#pragma once

#include "bt_types.hpp"

namespace BT
{

struct ConnectionState
{
	channel_mask_t channels_connected;

	/**
	 * Addresses for established connections by channel.
	 *
	 * address[0] is request address,
	 *            therefore only one request possible at a time.
	 * address[1-7] channel addresses if it is marked connected.
	 */
	Address6 address[8];

	/** Flag whether a new connection established or an existing one closed.
	 */
	bool changed;

	ConnectionState() : changed(false) {}
};

inline const Address6 &
get_address(const ConnectionState & self, channel_index_t ch)
{ return self.address[ch]; }

inline bool
allowed_connection_request(const ConnectionState & self)
{
	return (self.channels_connected.value & 0xfe) != 0xfe
		and not (self.channels_connected.value & 1);
}

inline void
register_connection_request(
	ConnectionState & self,
	const Address6 & remote_peer
) {
	self.changed = true;
	mark(self.channels_connected, 0, true);
	self.address[0] = remote_peer;
}

inline void
register_requested_connection(
	ConnectionState & self,
	channel_index_t ch
) {
	self.changed = true;
	mark(self.channels_connected, ch, true);
	self.address[ch] = self.address[0];
	mark(self.channels_connected, 0, false);
}

inline void
forget_connection_request(ConnectionState & self)
{
	/* Established connections have not changed. */
	mark(self.channels_connected, 0, false);
}

inline void
register_incoming_connection(
	ConnectionState & self,
	channel_index_t ch,
	const uint8_t (&bdAddr)[6]
) {
	self.changed = true;
	mark(self.channels_connected, ch, true);
	self.address[ch] = bdAddr;
}

inline void
register_disconnect(
	ConnectionState & self,
	channel_index_t ch
) {
	self.changed = true;
	mark(self.channels_connected, ch, false);
	self.address[ch] = Address6();
}

inline uint8_t
bitsum(uint8_t x)
{
	int n = 0;
	while (x != 0)
	{
		++n;
		x &= x - 1;
	}
	return n;
}

inline uint8_t
count_connections(const ConnectionState & self)
{ return bitsum(self.channels_connected.value & 0xfe); }

inline uint8_t
count_requests(const ConnectionState & self)
{ return self.channels_connected.value & 1; }

}
// end of namespace BT
