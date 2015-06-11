#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../debug.hpp"
#include "../svc_connections.hpp"

#include "connections.hpp"
#include "inquiry.hpp"
#include "pairing.hpp"
#include "service_defs.hpp"
#include "sync.hpp"

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
	InquiryState inq;
	SyncState sync;
    PairingState pairing;
};

template <typename Device>
bool
handle_service_packet(Device & dev, ServiceState & svc, const RESPONSE_EVENT_UNION & packet)
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
	processed =   handle(svc.inq, packet)    or processed;
    processed =   handle(dev, svc.pairing, packet) or processed;
	return processed;
}

inline bool
handle_unknown_packet(...) { return false; }

template <typename It, typename Size>
bool
handle_unknown_packet(ServiceState & svc, It first, Size n)
{
	bool processed = false;
	processed =   handle_unknown_packet(svc.sync, first, n)  or processed;
	processed =   handle_unknown_packet(svc.conn, first, n)  or processed;
	processed =   handle_unknown_packet(svc.inq,  first, n)  or processed;
	return processed;
}

inline bool
handle_inquiry_enhanced_data(...) { return false; }

template <typename It, typename Size>
bool
handle_inquiry_enhanced_data(ServiceState & svc, It first, Size n)
{
	bool processed = false;
	processed =   handle_inquiry_enhanced_data(svc.sync, first, n)  or processed;
	processed =   handle_inquiry_enhanced_data(svc.conn, first, n)  or processed;
	processed =   handle_inquiry_enhanced_data(svc.inq,  first, n)  or processed;
	return processed;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
