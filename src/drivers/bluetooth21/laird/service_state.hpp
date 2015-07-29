#pragma once

#include <cstdint>

#include "../bt_types.hpp"
#include "../debug.hpp"
#include "../svc_connections.hpp"
#include "../svc_globals.hpp"
#include "../trace.hpp"

#include "connections.hpp"
#include "inquiry.hpp"
#include "pairing.hpp"
#include "sdlog.hpp"
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
	PairingState pairing;
	SDLog sdlog;
	SyncState sync;

    GLOBAL_BT_STATE global_state;

	ServiceState() : sdlog(*this) {}
};

template <typename Device, typename State>
bool
handle_service_packet(ServiceBlockingIO< Device, State > & service_io, const RESPONSE_EVENT_UNION & packet)
{
	handle(service_io.state.flow, packet);
	bool processed = false;
	/*
	 * Try all handlers, but caller needs to know if the packet
	 * is known to at least one of them.
	 *
	 * FIXME Shouldn't we try only one of handlers?
	 */
	processed =   handle(service_io.state.sync, packet)   or processed;
	processed =   handle(service_io.state.conn, packet)   or processed;
	processed =   handle(service_io.state.inq, packet)    or processed;

    if (Globals::Service::get_pairing_status() == true) {
        // Preventing auto pairing messages. In the case when device have device B in its trusted db it will try to pair auto.
        processed =   handle(service_io, service_io.state.pairing, packet) or processed;
    }

	/*
	 * sdlog should be the last to reflect all the status changes.
	 */
	handle(service_io.state.sdlog, packet, processed);

	return processed;
}

template <typename PacketPOD>
inline void
on_write_command(ServiceState & svc, const PacketPOD & packet, bool ok)
{
	auto & u = *(const RESPONSE_EVENT_UNION*)&packet;
	on_write_command(svc.sdlog, u, ok);
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
	handle_unknown_packet(svc.sdlog, first, n, processed);
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
	handle_inquiry_enhanced_data(svc.sdlog, first, n, processed);
	return processed;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT

#include "sdlog.hpp"
