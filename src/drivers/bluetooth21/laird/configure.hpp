#pragma once

#include "../debug.hpp"
#include "../factory_addresses.hpp"
#include "../module_params.hpp"

#include "../std_util.hpp"

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
configure_name(ServiceIO & io)
{
#if defined(CONFIG_ARCH_BOARD_AIRDOG_FMU)
# define BT_LOCAL_NAME_PREFIX "Dog"
#elif defined(CONFIG_ARCH_BOARD_AIRLEASH)
# define BT_LOCAL_NAME_PREFIX "Leash"
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
				"%u-" BT_LOCAL_NAME_PREFIX "-%u",
				i, device_id);
		else
			l = snprintf(name, sizeof name,
				BT_LOCAL_NAME_PREFIX "-%u", device_id);

		ok = local_name_set(io, name, l)
			and local_name_store(io, name, l);
	}
	return ok;
#undef BT_LOCAL_NAME_PREFIX
#else
	return true;
#endif
}

template <typename ServiceIO>
bool
configure_before_reboot(ServiceIO & io)
{
	const uint32_t reg12 = Params::get("A_BT_S12_LINK");
	const uint32_t reg11 = Params::get("A_BT_S11_RFCOMM");
	const uint32_t reg80 = Params::get("A_BT_S80_LATENCY");
	const uint32_t reg84 = Params::get("A_BT_S84_POLL");

	struct SReg { uint32_t no, value; };
	SReg regs[] =
	{
		/* These registers require module reset */
		{  3, 1 },     // Profiles: SPP only
		{  6, 12 },    // Securty mode
		{ 12, reg12 }, // Link Supervision Timeout, seconds

		/*
		 * These registers impact module state after reset/reboot.
		 * Are used to detect that the module has rebooted.
		 */
		{  4, 0 },     // Default connectable: No
		{  5, 0 },     // Default discoverable: No

		/*
		 * These S-registers are set before reboot
		 * to save time on accidental module reboot
		 */
		{ 11, reg11 }, // RFCOMM frame size, bytes
		{ 14, 1 },     // Auto-accept connections
		{ 34, 1 },     // Number of incoming connections
		{ 35, 1 },     // Number of outgoing connections
		{ 80, reg80 }, // UART latency time in microseconds.
		{ 84, reg84 }, // UART poll mode
	};
	const size_t n_regs = sizeof(regs) / sizeof(*regs);

	bool ok = true;
	for (size_t i = 0; ok and i < n_regs; ++i)
		ok = s_register_set(io, regs[i].no, regs[i].value);

	if (ok) { ok = s_register_store(io); }

	dbg("configure_before_reboot");
	return ok;
}

template <typename ServiceIO>
bool
configure_after_reboot(ServiceIO & io, bool connectable)
{
	bool ok = ( configure_name(io)
		and switch_discoverable(io, true)
		and switch_connectable(io, true)
	);
	dbg("configure_after_reboot %i.\n", ok);
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
