#pragma once

#include <cstdint>

namespace BT
{
namespace Service
{
namespace Laird
{

/*
 * Laird's MP protocol definitions.
 * Drag them into our namespace despite half of them are defines.
 */
#include "BmHostProtocol.h"
#include "MpStatus.h"

using event_id_t = uint8_t;

inline event_id_t
get_event_id(const RESPONSE_EVENT_UNION & buf)
{
	auto & hdr = *(const BMEVENT_HDR*)&buf;
	return hdr.event;
}

template <typename PacketPOD>
inline auto
get_event_id(const PacketPOD & packet)
-> decltype(packet.hdr.command)
{ return packet.hdr.command; }

template <typename PacketPOD>
inline auto
get_event_id(const PacketPOD & packet)
-> decltype(packet.hdr.event)
{ return packet.hdr.event; }

inline bool
is_command(event_id_t x) { return 0 < x and x < 0x80; }

inline uint8_t
get_xt_flow(const RESPONSE_EVENT_UNION & buf)
{
	auto & hdr = *(const BMEVENT_HDR*)&buf;
	return hdr.flow << 1;
}

template <typename ResponsePOD>
inline uint8_t
get_response_status(const ResponsePOD & packet) { return packet.hdr.status; }

inline uint8_t
get_response_status(const RESPONSE_EVENT_UNION & buf)
{
	auto & hdr = *(const BMRESPONSE_HDR*)&buf;
	return hdr.status;
}

template <typename PacketType, event_id_t CMD_ID>
inline PacketType
prefill_packet()
{
	PacketType r;
	r.hdr.length = sizeof r;
	r.hdr.channel = 0;
	r.hdr.command = CMD_ID;
	r.hdr.flow = 0x7F;
	return r;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
