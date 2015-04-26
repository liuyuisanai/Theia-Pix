#pragma once

#include "bt_types.hpp"
#include "debug.hpp"

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
	bool ok = not (self.channels_connected.value & 1);
	if (ok)
	{
		ok = (self.channels_connected.value & 0xfe) != 0xfe;
		if (ok) { dbg("Connection request allowed.\n"); }
		else { dbg("All connection channels busy.\n"); }
	}
	else
	{
		dbg("Connection request to " Address6_FMT
			" still in progress.\n"
			, Address6_FMT_ITEMS(self.address[0])
		);
	}
	return ok;
}

inline void
register_connection_request(
	ConnectionState & self,
	const Address6 & remote_peer
) {
	self.changed = true;
	mark(self.channels_connected, 0, true);
	self.address[0] = remote_peer;
	dbg("register_connection_request 0x%02x " Address6_FMT ".\n"
		, self.channels_connected.value
		, Address6_FMT_ITEMS(self.address[0])
	);
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
	dbg("register_requested_connection 0x%02x"
		" channel %u address " Address6_FMT ".\n"
		, self.channels_connected.value
		, ch
		, Address6_FMT_ITEMS(self.address[ch])
	);
}

inline void
forget_connection_request(ConnectionState & self)
{
	/* Established connections have not changed. */
	mark(self.channels_connected, 0, false);
	dbg("forget_connection_request " Address6_FMT ".\n"
		, Address6_FMT_ITEMS(self.address[0])
	);
}

template <typename AddrT>
inline void
register_incoming_connection(
	ConnectionState & self,
	channel_index_t ch,
	const AddrT & addr
) {
	self.changed = true;
	mark(self.channels_connected, ch, true);
	self.address[ch] = addr;
	dbg("register_incoming_connection 0x%02x"
		" channel %u address " Address6_FMT ".\n"
		, self.channels_connected.value
		, ch
		, Address6_FMT_ITEMS(self.address[ch])
	);
}

inline void
register_disconnect(
	ConnectionState & self,
	channel_index_t ch
) {
	self.changed = true;
	mark(self.channels_connected, ch, false);
	dbg("register_disconnect 0x%02x"
		" channel %u address " Address6_FMT ".\n"
		, self.channels_connected.value
		, ch
		, Address6_FMT_ITEMS(self.address[ch])
	);
}

inline void
refresh_connections(ConnectionState & self, channel_mask_t active_channels)
{
	channel_mask_t connected = self.channels_connected;
	mark(connected, 0, false);

	channel_mask_t to_close = connected - active_channels;
	channel_mask_t to_open = active_channels - connected;

	dbg("refresh_connections: to close 0x%02x, to open 0x%02x\n",
		to_close.value, to_open.value);

	if (not empty(to_close))
	{
		self.changed = true;

		for (channel_index_t ch = 1; ch <= 7; ++ch)
		{
			/* Debug only cycle */
			if (is_set(to_open, ch))
			{
				dbg("disconnected channel %u"
					" address " Address6_FMT ".\n"
					, ch
					, Address6_FMT_ITEMS(self.address[ch])
				);
			}
		}

		self.channels_connected.value &= ~to_close.value;
	}

	if (not empty(to_open))
	{
		Address6 no_addr;
		for (channel_index_t ch = 1; ch <= 7; ++ch)
		{
			if (is_set(to_open, ch))
				register_incoming_connection(self, ch, no_addr);
		}
	}
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

inline bool
no_single_connection(const ConnectionState & self)
{ return (self.channels_connected.value & 0xfe) == 0; }

inline uint8_t
count_connections(const ConnectionState & self)
{ return bitsum(self.channels_connected.value & 0xfe); }

inline uint8_t
count_requests(const ConnectionState & self)
{ return self.channels_connected.value & 1; }

}
// end of namespace BT
