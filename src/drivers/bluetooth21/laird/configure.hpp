#pragma once

#include "../debug.hpp"
#include "../factory_addresses.hpp"
#include "../module_params.hpp"

#include "commands.hpp"
#include "service_defs.hpp"
#include "service_io.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

template <typename ServiceIO>
bool
configure_n_reboot(ServiceIO & io)
{
	uint32_t s12 = Params::get("A_BT_S12_LINK");

	bool as_is = s12 == BT_SREG_AS_IS;
	bool ok = ( s_register_set(io, 6, 12)
		and (as_is or s_register_set(io, 12, s12))
		and s_register_store(io)
		and soft_reset(io)
	);

	dbg("configure_n_reboot as-is %i ok %i.\n", as_is, ok);
	return ok;
}

template <typename ServiceIO>
bool
configure_name(ServiceIO & io)
{
#if defined(CONFIG_ARCH_BOARD_AIRDOG_FMU)
# define BT_LOCAL_NAME_PREFIX "AirDog"
#elif defined(CONFIG_ARCH_BOARD_AIRLEASH)
# define BT_LOCAL_NAME_PREFIX "AirLeash"
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V2)
# define BT_LOCAL_NAME_PREFIX "px4"
#endif

#ifdef BT_LOCAL_NAME_PREFIX
	Address6 addr;
	bool ok = local_address_read(io, addr);
	if (ok)
	{
		size_t i, l;
		i = find_n2(factory_addresses, n_factory_addresses, addr).second;

		char name[24];
		uint32_t device_id = Params::get("A_DEVICE_ID");
		if (i < n_factory_addresses)
			l = snprintf(name, sizeof name,
				BT_LOCAL_NAME_PREFIX "-F-%u-id-%u",
				i, device_id);
		else
			l = snprintf(name, sizeof name,
				BT_LOCAL_NAME_PREFIX "-%u", device_id);

		ok = local_name_set(io, name, l);
	}
	return ok;
#undef BT_LOCAL_NAME_PREFIX
#else
	return true;
#endif
}

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
	dbg("configure_general %i.\n", ok);
	return ok;
}

template <typename ServiceIO>
bool
configure_latency(ServiceIO & io)
{
	uint32_t s11 = Params::get("A_BT_S11_RFCOMM");
	uint32_t s80 = Params::get("A_BT_S80_LATENCY");
	uint32_t s84 = Params::get("A_BT_S84_POLL");

	bool ok = ( (s11 == BT_SREG_AS_IS or s_register_set(io, 11, s11))
		and (s80 == BT_SREG_AS_IS or s_register_set(io, 80, s80))
		and (s84 == BT_SREG_AS_IS or s_register_set(io, 84, s84))
	);
	dbg("configure_latency %i.\n", ok);
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

	Address6 local_addr;
	bool ok = local_address_read(io, local_addr);

	for (unsigned i = 0; i < n_factory_addresses; ++i)
		if (factory_addresses[i] != local_addr)
		{
			ok = add_trusted_key(io, factory_addresses[i], key);
			if (not ok) { break; }
		}

	return ok;
}

template <typename ServiceIO>
bool
configure_factory(ServiceIO & io)
{ return drop_trusted_db(io) and trust_factory(io); }

template <typename ServiceIO>
bool
dump_s_registers(ServiceIO & io)
{
	bool ok = true;
#ifdef CONFIG_DEBUG_BLUETOOTH21
	const uint8_t reg[] = {
		3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14,
		32, 33, 34, 35, 36, 37, 38, 40, 47,
		73, 74, 75, 76,
		80, 81, 82, 83, 84,
		240, 241, 242, 243,
		255
	};
	constexpr size_t n_reg = sizeof reg;
	uint32_t value[n_reg];
	size_t i, j;
	for (i = 0; ok and i < n_reg; ++i)
		ok = s_register_get(io, reg[i], value[i]);
	for (j = 0; j < i; ++j)
		dbg("SReg %3u (0x%02x) value %8u (0x%08x).\n",
			reg[j], reg[j], value[j], value[j]);
#endif // CONFIG_DEBUG_BLUETOOTH21
	return ok;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
