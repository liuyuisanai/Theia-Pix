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
 * @file landing.cpp
 *
 * Class for Handling Landing in Fixed WIng Position Control
 *
 * @author Thomas Gubler <thomasgubler@gmail.com>
 * @author Lorenz Meier <lm@inf.ethz.ch>
 */
#include <geo/geo.h>
#include <mathlib/mathlib.h>
#include <mavlink/mavlink_log.h>
#include <uORB/topics/tecs_status.h>

#include "landing.h"

namespace fwlanding
{
Landing::Landing(int mavlink_fd) :
	_landingslope(),
	_flare_curve_alt_rel_last(0.0f),
	_noreturn_vertical(false),
	_motor_lim(false),
	_stayonground(false),
	_onslope(false),
	_mavlink_fd(mavlink_fd)
	{}

void Landing::vertical(float throttle_land, float airspeed_land, float airspeed_approach, float terrain_alt,
		const struct position_setpoint_triplet_s &pos_sp_triplet, const math::Vector<2> &current_position,
		const math::Vector<2> &curr_wp,
		struct vehicle_global_position_s &global_pos,
		float wp_distance_save,
		float &throttle_max, // TODO throttle_max = _parameters.throttle_max;
		float throttle_land_max,
		float throttle_cruise,
		float bearing_lastwp_currwp,
		float bearing_airplane_currwp,
		float v_land, // TODO  = calculate_target_airspeed(airspeed_land);
		float v_approach, // TODO  = calculate_target_airspeed(airspeed_approach);
		float flare_pitch_min_rad,
		float flare_pitch_max_rad,
		float pitch_min_rad,
		float pitch_max_rad,
		struct landing_res_s & landing_res)

{
	float wp_distance = get_distance_to_next_waypoint(current_position(0), current_position(1), curr_wp(0), curr_wp(1));

	/* Calculate distance (to landing waypoint) and altitude of last ordinary waypoint L */
	float L_altitude_rel = pos_sp_triplet.previous.valid ?
		pos_sp_triplet.previous.alt - terrain_alt : 0.0f;

	float landing_slope_alt_rel_desired = _landingslope.getLandingSlopeRelativeAltitudeSave(wp_distance, bearing_lastwp_currwp, bearing_airplane_currwp);

	/*
	 * Check if we should start flaring with a vertical and a
	 * horizontal limit (with some tolerance)
	 * The horizontal limit is only applied when we are in front of the wp
	 */
	if (((global_pos.alt < terrain_alt + _landingslope.flare_relative_alt()) &&
				(wp_distance_save < _landingslope.flare_length() + 5.0f)) ||
			_noreturn_vertical) {  // checking for land_noreturn to avoid unwanted climb out
		/* land with minimal speed */

		/* force TECS to only control speed with pitch, altitude is only implicitely controlled now */
		// _tecs.set_speed_weight(2.0f);
		landing_res.speed_weight = 2.0f;

		/* kill the throttle if param requests it */

		if (global_pos.alt < terrain_alt + _landingslope.motor_lim_relative_alt() || _motor_lim) {
			throttle_max = math::min(throttle_max, throttle_land_max);
			if (!_motor_lim) {
				_motor_lim  = true;
				mavlink_log_info(_mavlink_fd, "#audio: Landing, limiting throttle");
			}

		}

		float flare_curve_alt_rel = _landingslope.getFlareCurveRelativeAltitudeSave(wp_distance, bearing_lastwp_currwp, bearing_airplane_currwp);

		/* avoid climbout */
		if ((_flare_curve_alt_rel_last < flare_curve_alt_rel && _noreturn_vertical) || _stayonground)
		{
			flare_curve_alt_rel = 0.0f; // stay on ground
			_stayonground = true;
		}

		// tecs_update_pitch_throttle(terrain_alt + flare_curve_alt_rel,
				// calculate_target_airspeed(airspeed_land), eas2tas,
				// math::radians(_parameters.land_flare_pitch_min_deg),
				// math::radians(_parameters.land_flare_pitch_max_deg),
				// 0.0f, throttle_max, throttle_land,
				// false,  land__motor_lim ? math::radians(_parameters.land_flare_pitch_min_deg) : math::radians(_parameters.pitch_limit_min),
				// global_pos.alt, ground_speed,
				// land__motor_lim ? tecs_status_s::TECS_MODE_LAND_THROTTLELIM : tecs_status_s::TECS_MODE_LAND);
		landing_res.alt_sp = terrain_alt + flare_curve_alt_rel;
		landing_res.v_sp = v_land;
		landing_res.pitch_min_rad = flare_pitch_min_rad;
		landing_res.pitch_max_rad = flare_pitch_max_rad;
		landing_res.throttle_min = 0.0f;
		landing_res.throttle_max = throttle_max;
		landing_res.throttle_cruise = throttle_land;
		landing_res.climbout_pitch_min_rad = _motor_lim ? flare_pitch_min_rad : pitch_min_rad;
		landing_res.tecs_status = _motor_lim ? tecs_status_s::TECS_MODE_LAND_THROTTLELIM : tecs_status_s::TECS_MODE_LAND;

		if (!_noreturn_vertical) {
			mavlink_log_info(_mavlink_fd, "#audio: Landing, flaring");
			_noreturn_vertical = true;
		}
		// warnx("Landing:  flare, global_pos.alt  %.1f, flare_curve_alt %.1f, flare_curve_alt_last %.1f, flare_length %.1f, wp_distance %.1f", global_pos.alt, flare_curve_alt, flare_curve_alt_last, flare_length, wp_distance);

		_flare_curve_alt_rel_last = flare_curve_alt_rel;
	} else {

		/*
		 * intersect glide slope:
		 * minimize speed to approach speed
		 * if current position is higher than the slope follow the glide slope (sink to the
		 * glide slope)
		 * also if the system captures the slope it should stay
		 * on the slope (bool land_onslope)
		 * if current position is below the slope continue at previous wp altitude
		 * until the intersection with slope
		 */

		float altitude_desired_rel;
		if (global_pos.alt > terrain_alt + landing_slope_alt_rel_desired || _onslope) {
			/* stay on slope */
			altitude_desired_rel = landing_slope_alt_rel_desired;
			if (!_onslope) {
				mavlink_log_info(_mavlink_fd, "#audio: Landing, on slope");
				_onslope = true;
			}
		} else {
			/* continue horizontally */
			altitude_desired_rel =  pos_sp_triplet.previous.valid ? L_altitude_rel :
				global_pos.alt - terrain_alt;
		}

       /*          tecs_update_pitch_throttle(terrain_alt + altitude_desired_rel, */
				// calculate_target_airspeed(airspeed_approach), eas2tas,
				// math::radians(_parameters.pitch_limit_min),
				// math::radians(_parameters.pitch_limit_max),
				// _parameters.throttle_min,
				// _parameters.throttle_max,
				// _parameters.throttle_cruise,
				// false,
				// math::radians(_parameters.pitch_limit_min),
				// global_pos.alt,
				/* ground_speed); */
		landing_res.alt_sp = terrain_alt + altitude_desired_rel;
		landing_res.v_sp = v_approach;
		landing_res.pitch_min_rad = pitch_min_rad;
		landing_res.pitch_max_rad = pitch_max_rad;
		landing_res.throttle_min = 0.0f;
		landing_res.throttle_max = throttle_max;
		landing_res.throttle_cruise = throttle_cruise;
		landing_res.climbout_pitch_min_rad = _motor_lim ? flare_pitch_min_rad : pitch_min_rad;
		landing_res.tecs_status = tecs_status_s::TECS_MODE_NORMAL;
	}

}


}
