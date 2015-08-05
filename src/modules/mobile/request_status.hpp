#pragma once

#include <uORB/topics/vehicle_global_position.h>
#include <uORB/topics/vehicle_status.h>

#include "protocol.h"

#include "request_base.hpp"
#include "uorb_base.hpp"

struct StatusOverall
{
	Subscription<vehicle_global_position_s, ORB_ID(vehicle_global_position)> gpos;
	Subscription<vehicle_status_s, ORB_ID(vehicle_status)> status;
};

static inline void
battery(const StatusOverall & self, StatusOverallReply & r)
{
	auto status = orb_read(self.status);
	r.battery_level = status.battery_remaining * 100.0f;
}

static inline void
position(const StatusOverall & self, StatusOverallReply & r)
{
	auto gpos = orb_read(self.gpos);
	r.gps_error_horizontal = gpos.eph;
	r.gps_error_vertical = gpos.epv;
}

static inline void
fill_reply(const StatusOverall & self, StatusOverallReply & r)
{
	battery(self, r);
	position(self, r);
}

template <>
struct Request< CMD_STATUS_OVERALL >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_STATUS_OVERALL >, Device & dev, const StatusOverall & status)
{
	StatusOverallReply r;
	fill_reply(status, r);
	write(dev, &r, sizeof r);
}
