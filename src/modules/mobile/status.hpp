#pragma once

#include <uORB/topics/vehicle_global_position.h>

#include "protocol.h"

#include "uorb_base.hpp"

struct StatusOverall
{
	Subscription<vehicle_global_position_s, ORB_ID(vehicle_global_position)> gpos;
};

StatusOverallReply
reply(StatusOverall & self)
{
	StatusOverallReply r;
	auto gpos = orb_read(self.gpos);
	r.gps_error_horizontal = gpos.eph;
	r.gps_error_vertical = gpos.epv;
	return std::move(r);
}
