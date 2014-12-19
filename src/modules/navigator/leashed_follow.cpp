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


Leashed::Leashed(Navigator *navigator, const char *name)
	: MissionBlock(navigator, name)
    , _v_module()
    , _target_lat()	
    , _target_lon()	
    , _target_alt()	
    , _target_v_n()
    , _target_v_e()
    , _target_v_module()
    , _vehicle_lat()	
    , _vehicle_lon()	
    , _vehicle_alt()
    , _init_alt() 		
    , _ready_to_follow(false)
    , _first_leash_point{0.0,0.0}
    , _last_leash_point{0.0,0.0}
{
    updateParameters();	
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
    // TODO [Max]: we need to exit this thing if points are zero
    bool ok1, ok2;
    double first_leash_point_nav[2];
    double last_leash_point_nav[2];
    ok1 = _navigator->get_path_points(0, first_leash_point_nav); // Get first point of path obtained in Lointer
    ok2 = _navigator->get_path_points(1, last_leash_point_nav); // Get last point of path obtained in Lointer
    if (ok1 && ok2) {
        _ready_to_follow = true;
        // Getting reference point for projection
		map_projection_init(&_ref_pos, global_pos->lat, global_pos->lon);
        // Projecting first point to local coords
        map_projection_project(
                &_ref_pos
                , first_leash_point_nav[0]
                , first_leash_point_nav[1]
                , &_first_leash_point[0]
                , &_first_leash_point[1]
                );
        // Projecting last point to local coords
        map_projection_project(
                &_ref_pos
                , last_leash_point_nav[0]
                , last_leash_point_nav[1]
                , &_last_leash_point[0]
                , &_last_leash_point[1]
                );
        // Constructing vector from first to last point in local coords
        _vector_v = math::Vector<2>(
                _last_leash_point[0] - _first_leash_point[0]
                ,_last_leash_point[1] - _first_leash_point[1]
                );
        _v_module = _vector_v.length();
        // Normalizing _vector_v
        _vector_v /= _v_module;

    } else
        _ready_to_follow = false;

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
        // Resulting vector
        math::Vector<2> vector_desired;

        /* we have at least 2 points setted from leash */
        _vehicle_lat = global_pos->lat;
        _vehicle_lon = global_pos->lon;
        _target_lat = target_pos->lat;
        _target_lon = target_pos->lon;
        _target_v_n = target_pos->vel_n;
        _target_v_e = target_pos->vel_e;

        
        // Vector from leash start path to vehicle
        float vehicle_x = 0.0;
        float vehicle_y = 0.0;
        map_projection_project(
                &_ref_pos
                , _vehicle_lat
                , _vehicle_lon
                , &vehicle_x
                , &vehicle_y
                );
        math::Vector<2> vector_vehicle(
                vehicle_x - _first_leash_point[0]
                , vehicle_y - _first_leash_point[1]
                );

        // Vector from start path to target
        float target_x = 0.0;
        float target_y = 0.0;
        map_projection_project(
                &_ref_pos
                , _target_lat
                , _target_lon
                , &target_x
                , &target_y
                );
        math::Vector<2> vector_target(
                target_x - _first_leash_point[0]
                , target_y - _first_leash_point[1]
                );

        // Calculating dot product of vehicle vector and path vector
        float vehicle_dot_product = _vector_v * vector_vehicle;

        // Limiting product not to be great than module of path vector
        float v_v_length = vector_vehicle.length();
        float from_vehicle_to_path = (v_v_length * v_v_length) - (vehicle_dot_product * vehicle_dot_product);

        /* --- if we are outside of path - return to it first -- */
        if ( from_vehicle_to_path > 3.0f*3.0f // TODO [Max] make this a param
             || vehicle_dot_product >= _v_module+3.0f
             || vehicle_dot_product < -3.0f
           ) { 
            //fprintf(stderr, "v_dot was %.3f ", (double) vehicle_dot_product);

            // Changing projection if vehicle outside of last/first points
            if (vehicle_dot_product >= _v_module) {
                vehicle_dot_product = _v_module;
            } else if (vehicle_dot_product < 0.0f) {
                vehicle_dot_product = 0.0f;
            }
            //fprintf(stderr, "Outside of path; to_path_sq %.3f, v_dot %.3f _vmod+3.0 %.3f\n"
            //        ,(double) from_vehicle_to_path
            //        ,(double) vehicle_dot_product
            //        ,(double)(_v_module+3.0f)
            //        );
            // Calculating vector from path start to desired point on path
            vector_desired = _vector_v * vehicle_dot_product;

            // ===== Resulting vector =====
            vector_desired -= vector_vehicle;
            //fprintf(stderr, " outside of path; vector_desired {%.3f,%.3f}\n"
            //        ,(double)vector_desired(0), (double)vector_desired(1)
            //       );
            pos_sp_triplet->current.velocity_valid = false;

        } else {
        /* -- We are on path and could follow target now -- */
            //Calculating dot product of target vector and path vector
            float target_dot_product = _vector_v * vector_target; 
            if (target_dot_product >= _v_module) {
                target_dot_product = _v_module;
                //fprintf(stderr, "On path, in last_point\n");
            } else if (target_dot_product < 0.0f) {
                target_dot_product = 0.0f;
                //fprintf(stderr, "On path, in first_point\n");
            } else {
                // Calculating velocity
                math::Vector<2> velocity_vector(_target_v_n, _target_v_e);
                velocity_vector = _vector_v * (velocity_vector * _vector_v);
                //fprintf(stderr, "velocity: target {%.3f,%.3f} vehicle{%.3f,%.3f}\n"
                //        ,(double) _target_v_n, (double) _target_v_e
                //        ,(double) velocity_vector(0), (double) velocity_vector(1)
                //       );
                pos_sp_triplet->current.vx = velocity_vector(0);
                pos_sp_triplet->current.vy = velocity_vector(1);
                pos_sp_triplet->current.velocity_valid = true;
            }


            // Calculating vector from path start to desired point on path
            vector_desired = _vector_v * target_dot_product;

            // ===== Resulting vector =====
            vector_desired -= vector_vehicle;
        }

        add_vector_to_global_position(
                _vehicle_lat
                , _vehicle_lon
                , vector_desired(0)
                , vector_desired(1)
                , &lat_new
                , &lon_new);
        

        pos_sp_triplet->current.valid = true;
        pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;

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
    else {
        // Switch back to LOITER
            commander_request_s *commander_request = _navigator->get_commander_request();
            commander_request->request_type = V_MAIN_STATE_CHANGE;
            commander_request->main_state = MAIN_STATE_LOITER;
            _navigator->set_commander_request_updated();
         }
}

void
Leashed::execute_vehicle_command() {
    // Update _parameter values with the latest navigator_mode parameters
    memcpy(&_parameters, &(NavigatorMode::_parameters), sizeof(_parameters));
	vehicle_command_s cmd = _vcommand;
	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;
		math::Vector<2> offset =_afollow_offset;
		switch(remote_cmd){
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}
            //case REMOTE_CMD_COME_TO_ME: {
            //     
            //     if (is_empty(_first_leash_point)) {
            //        /* setting current point as first point */
            //        global_pos = _navigator->get_global_position();
            //        _first_leash_point[0] = global_pos->lat;
            //        _first_leash_point[1] = global_pos->lon;
            //        _first_leash_point[2] = global_pos->alt;
            //        fprintf(stderr, "[leashed] Got point 1\n");
            //     }
            //     else if (is_empty(_last_leash_point)) {
            //        global_pos = _navigator->get_global_position();
            //        _last_leash_point[0] = global_pos->lat;
            //        _last_leash_point[1] = global_pos->lon;
            //        _last_leash_point[2] = global_pos->alt;
            //        _ready_to_follow = true;

            //        // Getting vector of the leashed path
            //        float v_x = 0.0;
            //        float v_y = 0.0;
            //        float v_z = _last_leash_point[2];
            //        get_vector_to_next_waypoint(
            //                _first_leash_point[0]
            //                , _first_leash_point[1]
            //                , _last_leash_point[0]
            //                , _last_leash_point[1]
            //                , &v_x
            //                , &v_y);

            //        // Normiruem vektor V
            //        _v_module = sqrt(v_x*v_x + v_y*v_y);
            //        v_x /= _v_module;
            //        v_y /= _v_module;
            //        v_z /= _v_module;

            //        _vector_v = math::Vector<3>(v_x, v_y, v_z);

            //        fprintf(stderr, "[leashed] Got point 2\n");
            //     }
            //     else {
            //         fprintf(stderr, "[leashed] Already have 2 points, ready: %d\n", _ready_to_follow);
            //     }
            //     break;
            // }
            //case REMOTE_CMD_LOOK_DOWN: {
            //     // Reseting vector poits
            //     _last_leash_point[0] = 0.0;
            //     _last_leash_point[1] = 0.0;
            //     _last_leash_point[2] = 0.0;
            //     _first_leash_point[0] = 0.0;
            //     _first_leash_point[1] = 0.0;
            //     _first_leash_point[2] = 0.0;
            //     _ready_to_follow = false;
            //     // Changing back ti Loiter state
			//	 commander_request_s *commander_request = _navigator->get_commander_request();
			//	 commander_request->request_type = V_MAIN_STATE_CHANGE;
			//	 commander_request->main_state = MAIN_STATE_LOITER;
			//	 _navigator->set_commander_request_updated();
            //     break;
            // }
		}
	}
}
