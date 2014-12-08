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
 * @file leashed_follow.cpp
 *
 * @author Max Shvetsov <max@airdog.com>
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
#include "leashed_follow.hpp"

static bool is_empty(double array[3]);

Leashed::Leashed(Navigator *navigator, const char *name)
	: MissionBlock(navigator, name)
    , _v_module()
    , _target_lat()	
    , _target_lon()	
    , _target_alt()	
    , _vehicle_lat()	
    , _vehicle_lon()	
    , _vehicle_alt()
    , _init_alt() 		
    , _ready_to_follow(false)
    , _first_leash_point{0.0,0.0,0.0}
    , _last_leash_point{0.0,0.0,0.0}
{
    updateParameters();	
    fprintf(stderr, "[leashed] Constructed ready: %d\n"
            ,_ready_to_follow);
}

Leashed::~Leashed()
{
}

void
Leashed::on_inactive()
{
}

void
Leashed::on_activation()
{
	updateParameters();
	global_pos = _navigator->get_global_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	pos_sp_triplet->next.valid = false;
	pos_sp_triplet->previous.valid = false;
	// Reset position setpoint to shoot and loiter until we get an acceptable trajectory point
	//pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
	//pos_sp_triplet->current.lat = global_pos->lat;
	//pos_sp_triplet->current.lon = global_pos->lon;
	//pos_sp_triplet->current.alt = global_pos->alt;
	//pos_sp_triplet->current.valid = true;
	//pos_sp_triplet->current.position_valid = true;
	//pos_sp_triplet->current.abs_velocity_valid = false;
	_navigator->set_position_setpoint_triplet_updated();
}

void
Leashed::on_active()
{
	// Execute command if received
	if ( update_vehicle_command() )
			execute_vehicle_command();

    target_pos = _navigator->get_target_position();
    global_pos = _navigator->get_global_position();
    double lat_new;
    double lon_new;

    if (_ready_to_follow) {
        /* we have at least 2 points setted from leash */
        _vehicle_lat = global_pos->lat;
        _vehicle_lon = global_pos->lon;
        _target_lat = target_pos->lat;
        _target_lon = target_pos->lon;

        
        // Vector from leash start path to vehicle
        float vehicle_x = 0.0;
        float vehicle_y = 0.0;
        float vehicle_z = _last_leash_point[2];
        get_vector_to_next_waypoint(
                _first_leash_point[0]
                , _first_leash_point[1]
                , _vehicle_lat
                , _vehicle_lon
                , &vehicle_x
                , &vehicle_y);

        // Vector from start path to target
        float target_x = 0.0;
        float target_y = 0.0;
        float target_z = _last_leash_point[2];
        get_vector_to_next_waypoint(
                _first_leash_point[0]
                , _first_leash_point[1]
                , _target_lat
                , _target_lon
                , &target_x
                , &target_y);

        // Calculating dot product of target vector and path vector
        float dot_product = 0.0f;
        math::Vector<3> vector_target(target_x, target_y, target_z);
        math::Vector<3> vector_vehicle(vehicle_x, vehicle_y, vehicle_z);
        dot_product = _vector_v * vector_target;

        // Limiting product not to be great than module of path vector
        if (dot_product >= _v_module) {
            dot_product = _v_module;
        } else if (dot_product < 0.0f) {
            dot_product = 0.0f;
        }

        // Calculating vector from path start to desired point on path
        math::Vector<3> vector_desired;
        vector_desired = _vector_v * dot_product;

        // ===== Resulting vector =====
        vector_desired -= vector_vehicle;
        fprintf(stderr, "[leashed] v_x: %.3f v_y: %.3f\n"
                ,(double) vector_desired(0)
                ,(double) vector_desired(1));

        add_vector_to_global_position(
                _vehicle_lat
                , _vehicle_lon
                , vector_desired(0)
                , vector_desired(1)
                , &lat_new
                , &lon_new);
    }
    else {
        // We still don't have points to follow, continuing to ABS follow
        _target_lat = target_pos->lat;
        _target_lon = target_pos->lon;
        fprintf(stderr, "[leashed] Flying like a ABS follow %.3f %.3f\n"
                , (double)target_pos->lat
                , (double)target_pos->lon);


        /* add offset to target position */
        add_vector_to_global_position(
                _target_lat
                , _target_lon
                , _afollow_offset(0)
                , _afollow_offset(1)
                , &lat_new
                , &lon_new);
    }

	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->current.type = SETPOINT_TYPE_MOVING;

	pos_sp_triplet->current.lat = lat_new;
	pos_sp_triplet->current.lon = lon_new;

	//if (_parameters.afol_rep_target_alt)
	//	pos_sp_triplet->current.alt = target_pos->alt - _afollow_offset(2);
	//else
	//	pos_sp_triplet->current.alt = _init_alt;


	/* calculate direction to target */
	pos_sp_triplet->current.yaw = get_bearing_to_next_waypoint(global_pos->lat, global_pos->lon, target_pos->lat, target_pos->lon);
	pos_sp_triplet->current.pitch_min = 0.0f;

	_navigator->set_position_setpoint_triplet_updated();
}

void
Leashed::execute_vehicle_command() {
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
            case REMOTE_CMD_COME_TO_ME: {
                 
                 if (is_empty(_first_leash_point)) {
                    /* setting current point as first point */
                    global_pos = _navigator->get_global_position();
                    _first_leash_point[0] = global_pos->lat;
                    _first_leash_point[1] = global_pos->lon;
                    _first_leash_point[2] = global_pos->alt;
                    fprintf(stderr, "[leashed] Got point 1\n");
                 }
                 else if (is_empty(_last_leash_point)) {
                    global_pos = _navigator->get_global_position();
                    _last_leash_point[0] = global_pos->lat;
                    _last_leash_point[1] = global_pos->lon;
                    _last_leash_point[2] = global_pos->alt;
                    _ready_to_follow = true;

                    // Getting vector of the leashed path
                    float v_x = 0.0;
                    float v_y = 0.0;
                    float v_z = _last_leash_point[2];
                    get_vector_to_next_waypoint(
                            _first_leash_point[0]
                            , _first_leash_point[1]
                            , _last_leash_point[0]
                            , _last_leash_point[1]
                            , &v_x
                            , &v_y);

                    // Normiruem vektor V
                    _v_module = sqrt(v_x*v_x + v_y*v_y);
                    v_x /= _v_module;
                    v_y /= _v_module;
                    v_z /= _v_module;

                    _vector_v = math::Vector<3>(v_x, v_y, v_z);

                    fprintf(stderr, "[leashed] Got point 2\n");
                 }
                 else {
                     fprintf(stderr, "[leashed] Already have 2 points, ready: %d\n", _ready_to_follow);
                 }
                 break;
             }
		}
	}
}
static bool is_empty(double array[3]) {
    return (array[0] == 0.0 && array[1] == 0.0 && array[2] == 0.0);
}
