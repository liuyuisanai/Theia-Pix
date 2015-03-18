#pragma once

#include "../module_params.hpp"
#include "../factory_addresses.hpp"

#include "commands.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

template <typename ServiceIO>
bool
configure_general(ServiceIO & io, bool connectable)
{
	bool ok = ( switch_discoverable(io, true)

		and switch_connectable(io, connectable)

		/* Auto-accept connections */
		and s_register_set(io, 14, connectable ? 1 : 0)

		/* Incoming connection number */
		and s_register_set(io, 34, 1)

		/* Outgoing connection number */
		and s_register_set(io, 35, 1)
	);
	return ok;
}

template <typename ServiceIO>
bool
configure_latency(ServiceIO & io)
{
	uint32_t s11 = Params::get("A_BT_S11_RFCOMM");
	uint32_t s12 = Params::get("A_BT_S12_LINK");
	uint32_t s80 = Params::get("A_BT_S80_LATENCY");
	uint32_t s84 = Params::get("A_BT_S84_POLL");

	bool ok = ( (s11 == BT_SREG_AS_IS or s_register_set(io, 11, s11))
		and (s12 == BT_SREG_AS_IS or s_register_set(io, 12, s12))
		and (s80 == BT_SREG_AS_IS or s_register_set(io, 80, s80))
		and (s84 == BT_SREG_AS_IS or s_register_set(io, 84, s84))
	);
	return ok;
}

template <typename ServiceIO>
bool
trust_factory(ServiceIO & io)
{
	LinkKey16 key(
		0xe8, 0x17, 0xfc, 0x99, 0xa2, 0xd0, 0x1b, 0x4b,
		0x07, 0xd2, 0xbb, 0xf9, 0xec, 0xba, 0x57, 0x9b
	);
	for (unsigned i = 0; i < n_factory_addresses; ++i)
	{
		bool ok = add_trusted_key(io, factory_addresses[i], key);
		if (not ok) { return false; }
	}
	return true;
}

template <typename ServiceIO>
bool
configure_factory(ServiceIO & io)
{ return drop_trusted_db(io) and trust_factory(io); }

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
