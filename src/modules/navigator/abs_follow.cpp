/****************************************************************************
 *
 *   Copyright (c) 2013-2014 PX4 Development Team. All rights reserved.
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
 * @file abs_follow.cpp
 *
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>

#include <mavlink/mavlink_log.h>
#include <systemlib/err.h>

#include <uORB/uORB.h>
#include <uORB/topics/position_setpoint_triplet.h>

#include "navigator.h"
#include "abs_follow.h"

AbsFollow::AbsFollow(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name)
{
    updateParameters();
}

AbsFollow::~AbsFollow()
{
}

void
AbsFollow::on_inactive()
{
}

void
AbsFollow::on_activation()
{
	updateParameters();

	_afollow_offset.zero();
	global_pos = _navigator->get_global_position();
	target_pos = _navigator->get_target_position();
	home_pos = _navigator->get_home_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	_init_alt = global_pos->alt;

	float target_alt = target_pos->alt;

	get_vector_to_next_waypoint_fast(target_pos->lat, target_pos->lon, global_pos->lat, global_pos->lon, &_afollow_offset.data[0], &_afollow_offset.data[1]);
	_afollow_offset.data[2] = -(global_pos->alt - target_alt);

	mavlink_log_info(_navigator->get_mavlink_fd(), "[abs_fol] Reset abs_follow offset: %.2f, %.2f, %.2f", (double)_afollow_offset(0), (double)_afollow_offset(1), (double)_afollow_offset(2));

	point_camera_to_target(&(pos_sp_triplet->current));
	_navigator->set_position_setpoint_triplet_updated();
}

void
AbsFollow::on_active()
{

	/* Update position data pointer */
	global_pos = _navigator->get_global_position();
	home_pos = _navigator->get_home_position();
	target_pos = _navigator->get_target_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	// Execute command if received
	if ( update_vehicle_command() )
			execute_vehicle_command();

	_target_lat = target_pos->lat;
	_target_lon = target_pos->lon;

	double lat_new;
	double lon_new;

	/* add offset to target position */
	add_vector_to_global_position(
			_target_lat, _target_lon,
			_afollow_offset(0), _afollow_offset(1),
			&lat_new, &lon_new
	);

	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->current.type = SETPOINT_TYPE_MOVING;

	pos_sp_triplet->current.lat = lat_new;
	pos_sp_triplet->current.lon = lon_new;

	if (_parameters.afol_rep_target_alt)
		pos_sp_triplet->current.alt = target_pos->alt - _afollow_offset(2);
	else
		pos_sp_triplet->current.alt = _init_alt;


	/* calculate direction to target */
	pos_sp_triplet->current.yaw = get_bearing_to_next_waypoint(global_pos->lat, global_pos->lon, target_pos->lat, target_pos->lon);
	pos_sp_triplet->current.pitch_min = 0.0f;

	_navigator->set_position_setpoint_triplet_updated();

}

void
AbsFollow::execute_vehicle_command() {

    // Update _parameter values with the latest navigator_mode parameters
    memcpy(&_parameters, &(NavigatorMode::_parameters), sizeof(_parameters));

	vehicle_command_s cmd = _vcommand;

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {

		REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;

		math::Vector<3> offset =_afollow_offset;

		switch(remote_cmd){
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}

			case REMOTE_CMD_UP: {

				_afollow_offset.data[2] -= _parameters.loi_step_len;
				break;
			}
			case REMOTE_CMD_DOWN: {

				_afollow_offset.data[2] += _parameters.loi_step_len;
				break;
			}
			case REMOTE_CMD_LEFT: {

				math::Matrix<3, 3> R_phi;

				double radius = sqrt(offset(0) * offset(0) + offset(1) * offset(1));

				// derived from formula: ( step / ( sqrt(x^2 + y^2)*2PI ) ) *  2PI
				// radius: (sqrt(x^2 + y^2)
				// circumference C: (radius * 2* PI)
				// step length fraction of C: step/C
				// angle of step fraction in radians: step/C * 2PI
				double alpha = (double)_parameters.loi_step_len / radius;

				// vector yaw rotation +alpha or -alpha depending on left or right
				R_phi.from_euler(0.0f, 0.0f, -alpha);
				math::Vector<3> offset_new  = R_phi * offset;

				_afollow_offset = offset_new;

				break;
			}
			case REMOTE_CMD_RIGHT: {

				math::Matrix<3, 3> R_phi;

				double radius = sqrt(offset(0) * offset(0) + offset(1) * offset(1));

				// derived from formula: ( step / ( sqrt(x^2 + y^2)*2PI ) ) *  2PI
				// radius: (sqrt(x^2 + y^2)
				// circumference C: (radius * 2* PI)
				// step length fraction of C: step/C
				// angle of step fraction in radians: step/C * 2PI
				double alpha = (double)_parameters.loi_step_len / radius;

				// vector yaw rotation +alpha or -alpha depending on left or right
				R_phi.from_euler(0.0f, 0.0f, alpha);
				math::Vector<3> offset_new  = R_phi * offset;

				_afollow_offset = offset_new;

				break;
			}
			case REMOTE_CMD_CLOSER: {

				// Calculate vector angle from target to device with atan2(y, x)
				float alpha = atan2f(offset(1), offset(0));

				// Create vector in the same direction, with loiter_step length
				math::Vector<3> offset_delta(
						cosf(alpha) * _parameters.loi_step_len,
						sinf(alpha) * _parameters.loi_step_len,
						0);

				math::Vector<3> offset_new = offset - offset_delta;

				_afollow_offset = offset_new;

				break;

			}

			case REMOTE_CMD_FURTHER: {

				// Calculate vector angle from target to device with atan2(y, x)
				float alpha = atan2(offset(1), offset(0));

				// Create vector in the same direction, with loiter_step length
				math::Vector<3> offset_delta(
						cosf(alpha) * _parameters.loi_step_len,
						sinf(alpha) * _parameters.loi_step_len,
						0);

				math::Vector<3> offset_new = offset + offset_delta;

				_afollow_offset = offset_new;

				break;
			}

		}
	}

}
