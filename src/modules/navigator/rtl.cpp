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
 * @file navigator_rtl.cpp
 * Helper class to access RTL
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <float.h>

#include <mavlink/mavlink_log.h>
#include <systemlib/err.h>
#include <geo/geo.h>

#include <uORB/uORB.h>
#include <uORB/topics/mission.h>
#include <uORB/topics/home_position.h>
#include <uORB/topics/position_setpoint_triplet.h>

#include "navigator.h"
#include "rtl.h"


RTL::RTL(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name)
{
    updateParameters();
	rtl_state = RTL_STATE_NONE;
}

RTL::~RTL()
{
}

void
RTL::on_inactive()
{
}

void
RTL::on_activation()
{
	updateParameters();

	_navigator->invalidate_setpoint_triplet();
	global_pos =  _navigator->get_global_position();
	home_pos = _navigator->get_home_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	first_rtl_setpoint_set = false;

	float accaptance_radius = _parameters.acceptance_radius;

	float xy_distance = get_distance_to_next_waypoint(
			global_pos->lat, global_pos->lon,
			home_pos->lat, home_pos->lon
	);


	/* Determine and set current RTL state */

	/* Vehicle have already landed */
	if (_navigator->get_vstatus()->condition_landed) {
		rtl_state = RTL_STATE_LANDED;
	/* Already home we only need to land */
	} else if (xy_distance <= accaptance_radius) {
		rtl_state = RTL_STATE_LAND;
		/* No need climb */
	} else if ( global_pos->alt >= home_pos->alt + _parameters.rtl_ret_alt) {
		rtl_state = RTL_STATE_RETURN;
	} else {
		rtl_state = RTL_STATE_CLIMB;
	}

	set_camera_mode(HORIZONTAL, true);

	set_rtl_setpoint();

}

void
RTL::on_active()
{

	mv_fd = _navigator->get_mavlink_fd();
	global_pos =  _navigator->get_global_position();
	home_pos = _navigator->get_home_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	if ( update_vehicle_command() )
			execute_vehicle_command();

	if (rtl_state != RTL_STATE_LANDED ) {

		if (!first_rtl_setpoint_set) {
			set_rtl_setpoint();
			first_rtl_setpoint_set = true;
		} else if (check_current_pos_sp_reached(rtl_state == RTL_STATE_LAND ? SETPOINT_TYPE_LAND : SETPOINT_TYPE_UNDEFINED)) {
			set_next_rtl_state();
			set_rtl_setpoint();
		}
	}

}

void
RTL::execute_vehicle_command()
{
    // Update _parameter values with the latest navigator_mode parameters

	vehicle_command_s cmd = _vcommand;

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		int remote_cmd = cmd.param1;
		if (remote_cmd == REMOTE_CMD_PLAY_PAUSE) {
			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_LOITER;
			if (rtl_state == RTL_STATE_LAND) {
				commander_request->mode_param = 1; // request takeoff
			}
			_navigator->set_commander_request_updated();
		}
	}

}

void
RTL::set_rtl_setpoint()
{

    pos_sp_triplet->previous.valid = false;
    pos_sp_triplet->current.valid = true;
    pos_sp_triplet->next.valid = false;
    
    float climb_alt = _navigator->get_home_position()->alt + _parameters.rtl_ret_alt;

	switch (rtl_state) {
		case RTL_STATE_CLIMB: {
			pos_sp_triplet->current.lat = global_pos->lat;
			pos_sp_triplet->current.lon = global_pos->lon;
			pos_sp_triplet->current.alt = climb_alt;
			pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
			pos_sp_triplet->current.position_valid = true;

			break;
		}
		case RTL_STATE_RETURN: {

            global_pos = _navigator->get_global_position();
            home_pos = _navigator->get_home_position();

            // Calculate offset values for later use.
            float offset_x;
            float offset_y;
            
            get_vector_to_next_waypoint(
                    global_pos->lat,
                    global_pos->lon,
                    home_pos->lat,
                    home_pos->lon,
                    &offset_x,
                    &offset_y
            );

            math::Vector<2> offset_xy(offset_x, offset_y);

            pos_sp_triplet->current.yaw = _wrap_pi(atan2f(offset_xy(1), offset_xy(0)));
            pos_sp_triplet->current.yaw_valid = true;

			pos_sp_triplet->current.lat = home_pos->lat;
			pos_sp_triplet->current.lon = home_pos->lon;
			//Do not reset altitude only if we were climbing before:
			if (fabsf(pos_sp_triplet->current.alt - climb_alt) >= FLT_EPSILON) {
				pos_sp_triplet->current.alt = global_pos->alt;
			}
			pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
			pos_sp_triplet->current.position_valid;

			break;
		}
		case RTL_STATE_LAND: {
			// TODO! [AK] Consider resetting Yaw to NaN to be coherent with other landing cases
            land(0); //do not reset setpoint, let it be at home position
			break;
		}
		case RTL_STATE_LANDED: {
			disarm();
			break;
		}
		default:
			break;
	}

	_navigator->set_position_setpoint_triplet_updated();

}

void
RTL::set_next_rtl_state()
{
	switch (rtl_state) {
		case RTL_STATE_CLIMB:
			rtl_state = RTL_STATE_RETURN;
			break;

		case RTL_STATE_RETURN:
			rtl_state = RTL_STATE_LAND;
			break;

		case RTL_STATE_LAND:
			rtl_state = RTL_STATE_LANDED;
			break;

		default:
			break;
	}
}
