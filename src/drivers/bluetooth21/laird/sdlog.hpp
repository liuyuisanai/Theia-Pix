#pragma once

#include <uORB/uORB.h>
#include <uORB/topics/bt21_laird.h>

#include "service_defs.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

struct ServiceState;

struct SDLog
{
	bt_svc_in_s svc_in;
	bt_svc_out_s svc_out;
	bt_evt_status_s evt_status;
	//SyncState::Level sync_level;

	orb_advert_t adv_svc_in;
	orb_advert_t adv_svc_out;
	orb_advert_t adv_evt_status;

	ServiceState & svc;

	SDLog(ServiceState & s)
	: svc(s)
	//: sync_level(SyncState::Level::UNKNOWN)
	{
		memset(&svc_in, 0, sizeof svc_in);
		memset(&svc_out, 0, sizeof svc_out);
		memset(&evt_status, 0, sizeof evt_status);

		adv_svc_in = orb_advertise(ORB_ID(bt_svc_in), &svc_in);
		adv_svc_out = orb_advertise(ORB_ID(bt_svc_out), &svc_out);
		adv_evt_status = orb_advertise(ORB_ID(bt_evt_status), &evt_status);
	}
};

inline void
publish(const SDLog & self, const bt_svc_in_s & s)
{ orb_publish(ORB_ID(bt_svc_in), self.adv_svc_in, &s); }

inline void
publish(const SDLog & self, const bt_svc_out_s & s)
{ orb_publish(ORB_ID(bt_svc_out), self.adv_svc_out, &s); }

inline void
publish(const SDLog & self, const bt_evt_status_s & s)
{ orb_publish(ORB_ID(bt_evt_status), self.adv_evt_status, &s); }

template <typename SvcState>
// Template is a hack to make it compile.
void
log_service_status(SDLog & self, SvcState & svc)
{
	if (svc.conn.changed)
	{
		self.evt_status.channels_connected = svc.conn.channels_connected.value;
		publish(self, self.evt_status);
	}
}

inline void
handle(SDLog & self, const RESPONSE_EVENT_UNION & p, bool processed)
{
	//auto & hdr = *(const BMEVENT_HDR *)&p;
	//self.svc_in.length = hdr.length;
	//self.svc_in.channel = hdr.channel;
	//self.svc_in.cmd_evt_id = hdr.event;
	//self.svc_in.flow = hdr.flow;
	*(BMEVENT_HDR *)&self.svc_in = *(const BMEVENT_HDR *)&p;
	self.svc_in.cmd_status = is_command(get_event_id(p)) ? get_response_status(p) : 0;
	self.svc_in.processed = processed;
	publish(self, self.svc_in);

	if (get_event_id(p) == EVT_STATUS)
	{
		self.evt_status.status = p.evtStatus.status;
		self.evt_status.discoverable_mode = p.evtStatus.discoverable_mode;
		self.evt_status.connectable_mode = p.evtStatus.connectable_mode;
		self.evt_status.security_mode = p.evtStatus.security_mode;
		publish(self, self.evt_status);
	}

	log_service_status(self, self.svc);
}

inline void
on_write_command(SDLog & self, const RESPONSE_EVENT_UNION & p, bool ok)
{
	self.svc_out.cmd = get_event_id(p);
	self.svc_out.ok = ok;
	publish(self, self.svc_out);
}

template <typename It, typename Size>
void
handle_unknown_packet(SDLog & self, It first, Size n, bool processed)
{
	uint8_t * p = (uint8_t*)&self.svc_in;
	fill(copy_n(first, min(n, 5u), p), p + 5, 0);
	self.svc_in.processed = processed;
	publish(self, self.svc_in);

	log_service_status(self, self.svc);
}

template <typename It, typename Size>
void
handle_inquiry_enhanced_data(SDLog & self, It first, Size n, bool processed)
{ log_service_status(self, self.svc); }

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
