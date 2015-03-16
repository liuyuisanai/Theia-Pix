#pragma once

#include "../debug.hpp"
#include "../network_util.hpp"
#include "../time.hpp"

#include "service_defs.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

template <uint8_t CMD_ID, typename ServiceIO>
bool
send_simple_command(ServiceIO & io)
{
	RESPONSE_SIMPLE rsp;
	auto cmd = prefill_packet<COMMAND_SIMPLE, CMD_ID>();

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;

	dbg("-> command simple_command() %s.\n", ok ? "ok": "failed");
	return ok;
}

template <typename ServiceIO>
bool
module_factory_default(ServiceIO & io, uint8_t flagmask)
{
	RESPONSE_FACTORYDEFAULT rsp;
	auto cmd =
		prefill_packet<COMMAND_FACTORYDEFAULT, CMD_FACTORYDEFAULT>();
	cmd.flagmask = flagmask | 0b00111000;

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;

	dbg("-> command module_factory_default(0x%02x) %s.\n",
		flagmask, ok ? "ok": "failed");
	return ok;
}

template <typename ServiceIO>
bool
s_register_get(ServiceIO & io, uint32_t regno, uint32_t & value)
{
	RESPONSE_READ_SREG rsp;
	auto cmd = prefill_packet<COMMAND_READ_SREG, CMD_READ_SREG>();
	cmd.regNo = (uint8_t)regno;

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;
	if (ok) { value = network_to_host(rsp.regVal); }

	dbg("-> command s_register_get(%u) %s -> %u.\n"
		, regno
		, ok ? "ok": "failed"
		, value);
	return ok;
}

template <typename ServiceIO>
bool
s_register_set(ServiceIO & io, uint32_t regno, uint32_t value)
{
	RESPONSE_WRITE_SREG rsp;
	auto cmd = prefill_packet<COMMAND_WRITE_SREG, CMD_WRITE_SREG>();
	cmd.regNo = (uint8_t)regno;
	host_to_network(value, cmd.regVal);

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;

	dbg("-> command s_register_set(%u, %u) %s.\n"
		, regno
		, value
		, ok ? "ok": "failed");
	return ok;
}

template <typename ServiceIO>
bool
switch_connectable(ServiceIO & io, bool enable)
{
	RESPONSE_CONNECTABLE_MODE rsp;
	auto cmd = prefill_packet<COMMAND_CONNECTABLE_MODE,
				  CMD_CONNECTABLE_MODE>();
	cmd.enable = (uint8_t)(enable ? 1 : 0);
	cmd.autoAccept = 0;

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK
		and rsp.currentMode == cmd.enable;

	dbg("-> command switch_connectable(%i) %s.\n"
		, enable
		, ok ? "ok": "failed");
	return ok;
}

template <typename ServiceIO>
bool
switch_discoverable(ServiceIO & io, bool enable)
{
	RESPONSE_DISCOVERABLE_MODE rsp;
	auto cmd = prefill_packet<COMMAND_DISCOVERABLE_MODE,
				  CMD_DISCOVERABLE_MODE>();
	cmd.enable = (uint8_t)(enable ? 1 : 0);

	bool ok = send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK
		and rsp.currentMode == cmd.enable;

	dbg("-> command switch_discoverable(%i) %s.\n"
		, enable
		, ok ? "ok": "failed");
	return ok;
}

// TODO Address6
//template <typename ServiceIO>
//bool
//add_trusted_key(ServiceIO & io, const Address6 & addr, const LinkKey16 & key)
//{
//	RESPONSE_TRUSTED_DB_ADD rsp;
//	auto cmd =
//		prefill_packet<COMMAND_TRUSTED_DB_ADD, CMD_TRUSTED_DB_ADD>();
//	copy(cbegin(addr), cend(addr), cmd.bdAddr);
//	copy(cbegin(key), cend(key), cmd.linkKey);
//	fill(cbegin(cmd.keyFlags), cend(cmd.keyFlags), 0);
//
//	bool ok = send_receive_verbose(io, cmd, rsp)
//		and get_response_status(rsp) == MPSTATUS_OK;
//	dbg("-> command add_trusted_key %s.\n", ok ? "ok": "failed");
//	return ok;
//}

template <typename ServiceIO>
bool
drop_trusted_db(ServiceIO & io)
{
	bool ok = module_factory_default(io, 1 << 6);
	dbg("-> command drop_trusted_db() %s.\n", ok ? "ok": "failed");
	return ok;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
