#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../connection_state.hpp"
#include "../debug.hpp"

#include "connections.hpp"
#include "sync.hpp"
#include "service_defs.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

struct FlowState
{
	channel_mask_t xt_mask;
};

inline void
handle(FlowState & self, const RESPONSE_EVENT_UNION & packet)
{ self.xt_mask = get_xt_flow(packet); }

struct ServiceState
{
	ConnectionState conn;
	FlowState flow;
	SyncState sync;
};

bool
handle_service_packet(ServiceState & svc, const RESPONSE_EVENT_UNION & packet)
{
	handle(svc.flow, packet);
	bool processed = false;
	/*
	 * Try all handlers, but caller needs to know if the packet
	 * is known to at least one of them.
	 *
	 * FIXME Shouldn't we try only one of handlers?
	 */
	processed =   handle(svc.sync, packet)   or processed;
	processed =   handle(svc.conn, packet)   or processed;
	return processed;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
