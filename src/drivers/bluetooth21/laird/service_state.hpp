#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../connection_state.hpp"
#include "../debug.hpp"

#include "connections.hpp"
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
};

bool
handle_service_packet(ServiceState & svc, const RESPONSE_EVENT_UNION & packet)
{
	handle(svc.flow, packet);
	return handle(svc.conn, packet);
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
