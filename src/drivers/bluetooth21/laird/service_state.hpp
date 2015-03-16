#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../debug.hpp"

#include "service_defs.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

struct ServiceState
{
	bool xt_flow_changed;
	channel_mask_t xt_flow;
};

void
process_event(ServiceState & svc, const RESPONSE_EVENT_UNION & packet)
{
	auto event_id = get_event_id(packet);

	switch (get_event_id(packet))
	{
	// case CMD_CHANNEL_LIST:
	// 	if (get_response_status() == MPSTATUS_OK)
	// 		receive_open_channels(self, packet.rspChannelList);
	// break;

	// case CMD_DROP_CONNECTION:
	// 	/* hostHandle is channelId, see request_disconnect() */
	// 	if (get_response_status() == MPSTATUS_OK)
	// 	{
	// 		register_disconnect(
	// 			svc.conns,
	// 			packet.rspDropConnection.hostHandle
	// 		);
	// 	}
	// break;

	// case CMD_MAKE_CONNECTION:
	// 	if (get_response_status() == MPSTATUS_OK)
	// 	{
	// 		channel_index_t i = packet.rspMakeConnection.channelId;
	// 		register_connection_out<ChannelMode::ESTABLISHED>(svc.conns, i);
	// 		// TODO convert to hex12 only when debug enabled.
	// 		hex12_str_t hex12;
	// 		hex12_str(get_address(svc.conns, i), hex12);
	// 		dbg("-> CMD_MAKE_CONNECTION: established connection"
	// 			" at channel %i to %s.\n" , i , hex12);
	// 	}
	// 	// TODO turn into a callback
	// 	reset(svc.conns.request_address);
	// break;

	// case EVT_DISCONNECT:
	// 	register_disconnect(
	// 		svc.conns,
	// 		packet.evtDisconnect.channelId
	// 	);
	// 	dbg("-> EVT_DISCONNECT: at channel %i reason 0x%02x.\n"
	// 		, packet.evtDisconnect.channelId
	// 		, packet.evtDisconnect.reason
	// 	);
	// break;

	// case EVT_INCOMING_CONNECTION:
	// 	// TODO check channel number
	// 	if (network_to_host(packet.evtIncomingConnection.uuid) != UUID_SPP)
	// 	{
	// 		dbg("-> EVT_INCOMING_CONNECTION: unsupported"
	// 			" uuid 0x%04x.\n", network_to_host(
	// 				packet.evtIncomingConnection.uuid
	// 			)
	// 		);
	// 	}
	// 	else
	// 	{
	// 		channel_index_t i = packet.evtIncomingConnection.channelId;
	// 		register_connection_in<ChannelMode::ESTABLISHED>(
	// 			svc.conns,
	// 			i,
	// 			make_address6(packet.evtIncomingConnection.bdAddr)
	// 		);
	// 		// TODO convert to hex12 only when debug enabled.
	// 		hex12_str_t hex12;
	// 		hex12_str(get_address(svc.conns, i), hex12);
	// 		dbg("-> EVT_INCOMING_CONNECTION:"
	// 			" established connection"
	// 			" at channel %i to %s.\n", i, hex12);
	// 	}
	// break;

	case EVT_STATUS:
		dbg("-> EVT_STATUS: %d disco %d conn %d sec %d.\n"
			, packet.evtStatus.status
			, packet.evtStatus.discoverable_mode
			, packet.evtStatus.connectable_mode
			, packet.evtStatus.security_mode
		);
	break;

	case EVT_UNKNOWN_COMMAND:
		dbg("-> EVT_UNKNOWN_COMMAND: command id 0x%02x.\n",
			packet.evtUnknownCmd.command);
	break;

	default:
		if (not is_command(event_id))
			dbg("-> Event 0x%02x dropped.\n", event_id);
	break;
	}
}

void
dbg_responce_received(const RESPONSE_EVENT_UNION & packet)
{
#ifdef CONFIG_DEBUG_BLUETOOTH21
	auto event_id = get_event_id(packet);
	auto status = get_response_status(packet);
	if (status == MPSTATUS_OK)
		dbg("-> CMD 0x%02x OK\n", event_id);
	else
		dbg("-> CMD 0x%02x ERROR 0x%02x\n", event_id, status);
#endif
}

void
process_service_packet(ServiceState & svc, const RESPONSE_EVENT_UNION & packet)
{
	svc.xt_flow_changed = svc.xt_flow.value != get_xt_flow(packet);
	if (svc.xt_flow_changed) { svc.xt_flow.value = get_xt_flow(packet); }

	auto event_id = get_event_id(packet);
	if (is_command(event_id)) { dbg_responce_received(packet); }

	process_event(svc, packet);
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
