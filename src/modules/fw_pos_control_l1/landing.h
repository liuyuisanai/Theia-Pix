/****************************************************************************
 *
 *   Copyright (c) 2013 - 2015 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/


/**
 * @file landing.h
 *
 * Class for Handling Landing in Fixed WIng Position Control
 *
 * @author Thomas Gubler <thomasgubler@gmail.com>
 * @author Lorenz Meier <lm@inf.ethz.ch>
 */
#pragma once

#include "landingslope.h"
#include <uORB/topics/vehicle_global_position.h>

namespace fwlanding
{

struct __EXPORT landing_vertical_res_s
{
	float alt_sp;
	float v_sp;
	float pitch_min_rad;
	float pitch_max_rad;
	float throttle_min;
	float throttle_max;
	float throttle_cruise;
	float climbout_pitch_min_rad;
	unsigned tecs_status;
	float speed_weight;

};

enum LANDING_HORIZONTAL_NAVMODE {
	LANDING_HORIZONTAL_NAVMODE_NORMAL,
	LANDING_HORIZONTAL_NAVMODE_HEADING,
};

struct __EXPORT landing_horizontal_res_s
{
	enum LANDING_HORIZONTAL_NAVMODE navmode;
	float target_bearing;
	bool constrain_roll;
};

class __EXPORT Landing
{
public:
	Landing(int mavlink_fd);
	~Landing();

	void vertical(float throttle_land, float airspeed_land, float airspeed_approach, float terrain_alt,
		const struct position_setpoint_triplet_s &pos_sp_triplet, const math::Vector<2> &current_position,
		const math::Vector<2> &curr_wp,
		struct vehicle_global_position_s &global_pos,
		float wp_distance_save,
		float &throttle_max, // TODO throttle_max = _parameters.throttle_max;
		float throttle_land_max,
		float throttle_cruise,
		float bearing_lastwp_currwp,
		float bearing_airplane_currwp,
		float v_land, // TODO v_sp = calculate_target_airspeed(airspeed_land);
		float v_approach, // TODO v_sp = calculate_target_airspeed(airspeed_approach);
		float flare_pitch_min_rad,
		float flare_pitch_max_rad,
		float pitch_min_rad,
		float pitch_max_rad,
		struct landing_vertical_res_s &landing_res);

void horizontal(const math::Vector<2> &current_position,
		const math::Vector<2> &curr_wp,
		float bearing_lastwp_currwp,
		float bearing_airplane_currwp,
		float heading_hold_horizontal_distance,
		const struct position_setpoint_triplet_s &pos_sp_triplet,
		const struct vehicle_attitude_s &att,
		struct landing_horizontal_res_s &landing_res
		);

private:
	Landingslope _landingslope;
	float _flare_curve_alt_rel_last;
	bool _noreturn_vertical;
	bool _noreturn_horizontal;
	bool _motor_lim;
	bool _stayonground;
	bool _onslope;
	float _target_bearing;
	int _mavlink_fd;

};

}
