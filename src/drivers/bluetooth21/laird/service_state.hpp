#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../connection_state.hpp"
#include "../debug.hpp"
#include "../network_util.hpp"

#include "../std_algo.hpp"
#include "../std_iter.hpp"

#include "service_defs.hpp"
#include "service_io.hpp"

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
	ConnectionState conn;

	ServiceState()
	: xt_flow_changed(false)
	{}
};

template <typename Device>
bool
request_connect(Device & dev, ServiceState & svc, const Address6 & addr)
{
	if (not allowed_connection_request(svc.conn))
		return false;

	auto cmd =
		prefill_packet<COMMAND_MAKE_CONNECTION, CMD_MAKE_CONNECTION>();
	cmd.hostHandle = 0;
	copy(begin(addr), end(addr), cmd.bdAddr);
	host_to_network(uint16_t(UUID_SPP), cmd.uuid);
	cmd.instanceIndex = 0;

	bool ok = write_command(dev, &cmd, sizeof cmd);

	if (ok) { register_connection_request(svc.conn, addr); }

	dbg("-> request MAKE_CONNECTION(" Address6_FMT ") %s.\n"
		, Address6_FMT_ITEMS(addr)
		, ok ? "ok": "failed");
	return ok;
}

void
process_event(ServiceState & svc, const RESPONSE_EVENT_UNION & p)
{
	event_id_t event_id = get_event_id(p);
	switch (get_event_id(p))
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
	// 			svc.conn,
	// 			p.rspDropConnection.hostHandle
	// 		);
	// 	}
	// break;

	case CMD_MAKE_CONNECTION:
		if (get_response_status(p) == MPSTATUS_OK)
		{
			channel_index_t ch = p.rspMakeConnection.channelId;
			register_requested_connection(svc.conn, ch);

			const Address6 & addr = get_address(svc.conn, ch);
			dbg("-> MAKE_CONNECTION:"
				" Channel %u got connected connection"
				" to " Address6_FMT ".\n"
				, ch
				, Address6_FMT_ITEMS(addr)
			);
		}
		else
		{
			forget_connection_request(svc.conn);
			dbg("-> MAKE_CONNECTION failed with status 0x%02x.\n",
				get_response_status(p));
		}
	break;

	case EVT_DISCONNECT:
		if (p.evtDisconnect.channelId > 7)
			dbg("-> EVT_DISCONNECT: Invalid channel %u.\n"
				, p.evtDisconnect.channelId
			);
		else
		{
			channel_index_t ch = p.evtDisconnect.channelId;
			register_disconnect(svc.conn, ch);
			dbg("-> EVT_DISCONNECT: at channel %u reason 0x%02x.\n"
				, ch
				, p.evtDisconnect.reason
			);
		}
	break;

	case EVT_INCOMING_CONNECTION:
		if (network_to_host(p.evtIncomingConnection.uuid) != UUID_SPP
		or p.evtIncomingConnection.channelId > 7
		) {
			channel_index_t ch = p.evtIncomingConnection.channelId;
			auto uuid =
				network_to_host(p.evtIncomingConnection.uuid);
			dbg("-> EVT_INCOMING_CONNECTION: Error"
				" unsupported uuid 0x%04x at channel %u.\n"
				, uuid
				, ch
			);
		}
		else
		{
			channel_index_t ch = p.evtIncomingConnection.channelId;
			register_incoming_connection(
				svc.conn, ch, p.evtIncomingConnection.bdAddr
			);
			const Address6 & addr = get_address(svc.conn, ch);
			dbg("-> EVT_INCOMING_CONNECTION:"
				" Channel %u got connected"
				" to " Address6_FMT ".\n"
				, ch
				, Address6_FMT_ITEMS(addr)
			);
		}
	break;

	case EVT_STATUS:
		dbg("-> EVT_STATUS: %d disco %d conn %d sec %d.\n"
			, p.evtStatus.status
			, p.evtStatus.discoverable_mode
			, p.evtStatus.connectable_mode
			, p.evtStatus.security_mode
		);
	break;

	case EVT_UNKNOWN_COMMAND:
		dbg("-> EVT_UNKNOWN_COMMAND: command id 0x%02x.\n",
			p.evtUnknownCmd.command);
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
