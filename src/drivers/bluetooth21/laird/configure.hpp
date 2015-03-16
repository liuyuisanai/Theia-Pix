#pragma once

#include "../module_params.hpp"

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

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
