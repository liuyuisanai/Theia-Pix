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

template <typename packet_type, event_id_t cmd_id>
inline packet_type
prefill_packet()
{
	packet_type r;
	r.hdr.length = sizeof r;
	r.hdr.channel = 0;
	r.hdr.command = cmd_id;
	r.hdr.flow = 0x7F;
	return r;
}

template <uint8_t CMD_ID, typename ServiceIO>
bool
send_simple_command(ServiceIO & io)
{
	RESPONSE_SIMPLE rsp;
	auto cmd = prefill_packet<COMMAND_SIMPLE, CMD_ID>();

	return send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;
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

	return send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;
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

	return send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK
		and rsp.currentMode == cmd.enable;
}

template <typename ServiceIO>
bool
switch_discoverable(ServiceIO & io, bool enable)
{
	RESPONSE_DISCOVERABLE_MODE rsp;
	auto cmd = prefill_packet<COMMAND_DISCOVERABLE_MODE,
				  CMD_DISCOVERABLE_MODE>();
	cmd.enable = (uint8_t)(enable ? 1 : 0);

	return send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK
		and rsp.currentMode == cmd.enable;
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
//	return send_receive_verbose(io, cmd, rsp)
//		and get_response_status(rsp) == MPSTATUS_OK;
//}

template <typename ServiceIO>
bool
drop_trusted_db(ServiceIO & io)
{
	RESPONSE_FACTORYDEFAULT rsp;
	auto cmd =
		prefill_packet<COMMAND_FACTORYDEFAULT, CMD_FACTORYDEFAULT>();
	cmd.flagmask = 0b01111000;

	return send_receive_verbose(io, cmd, rsp)
		and get_response_status(rsp) == MPSTATUS_OK;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
