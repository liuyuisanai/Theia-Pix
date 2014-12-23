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
    , _vehicle_v_n()
    , _vehicle_v_e()
    , _vehicle_lat()	
    , _vehicle_lon()	
    , _vehicle_alt()
    , _init_alt() 		
    , _ready_to_follow(false)
    , _first_leash_point{0.0,0.0}
    , _last_leash_point{0.0,0.0}
    , _t_prev()
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
    pos_sp_triplet->current.valid = false;
    pos_sp_triplet->previous.valid = false;
    pos_sp_triplet->next.valid = false;

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

    } else {
        _ready_to_follow = false;
        // Switch back to LOITER
        commander_request_s *commander_request = _navigator->get_commander_request();
        commander_request->request_type = V_MAIN_STATE_CHANGE;
        commander_request->main_state = MAIN_STATE_LOITER;
        _navigator->set_commander_request_updated();
    }

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
        _vehicle_v_n = global_pos->vel_n;
        _vehicle_v_e = global_pos->vel_e;
        _target_v_n = target_pos->vel_n;
        _target_v_e = target_pos->vel_e;

        // Calculating time for feed forward
        hrt_abstime target_time = target_pos->timestamp;
        float dt = 0.0f;
        if (_t_prev != 0) {
		    dt = target_time != 0 ? (target_time - _t_prev) * 0.000001f : 0.0f;
        } else {
            // First use
            dt = 0.0f;
        }
        _t_prev = target_time; // Setting this as previous time

        
        // Vector from leash start path to vehicle
        float vehicle_x = 0.0f;
        float vehicle_y = 0.0f;
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
        if ( from_vehicle_to_path > _parameters.acceptance_radius * _parameters.acceptance_radius
             || vehicle_dot_product >= _v_module + _parameters.acceptance_radius
             || vehicle_dot_product < -_parameters.acceptance_radius
           ) { 

            // Changing projection if vehicle outside of last/first points
            if (vehicle_dot_product >= _v_module) {
                vehicle_dot_product = _v_module;
            } else if (vehicle_dot_product < 0.0f) {
                vehicle_dot_product = 0.0f;
            }
            // Calculating vector from path start to desired point on path
            vector_desired = _vector_v * vehicle_dot_product;

            // ===== Resulting vector =====
            vector_desired -= vector_vehicle;
            pos_sp_triplet->current.velocity_valid = false;

        } else {
        /* -- We are on path and could follow target now -- */
            //Calculating dot product of target vector and path vector
            float target_dot_product = _vector_v * vector_target; 
            if (target_dot_product >= _v_module) {
                target_dot_product = _v_module;
            } else if (target_dot_product < 0.0f) {
                target_dot_product = 0.0f;
            } else {
                // Calculating velocity
                math::Vector<2> velocity_vector(_target_v_n, _target_v_e);
                math::Vector<2> current_velocity_vector(_vehicle_v_n, _vehicle_v_e);
                float current_velocity = current_velocity_vector.length();
                float required_velocity = velocity_vector * _vector_v;
                // Calculating required velocity change module
                float velocity_change = required_velocity - current_velocity;
                if (dt != 0.0f) {
                    if (fabsf(velocity_change) / dt > _parameters.acceleration) {
                        // We want to accelerate more than it is allowed, limit speed
                        if (velocity_change > 0.0f) {
                            required_velocity = current_velocity + dt * _parameters.acceleration;
                        } else {
                            required_velocity = current_velocity - dt * _parameters.acceleration;
                        }
                    }
                }


                velocity_vector = _vector_v * required_velocity;
                
                float dist_to_max_point = _v_module - vehicle_dot_product;
                float current_allowed_velocity;
                // min(dist to first; dist to last)
                if (dist_to_max_point > vehicle_dot_product && vehicle_dot_product > target_dot_product) {
                    //We are near first point and comming to it
                    current_allowed_velocity = (vehicle_dot_product - 3.0f) * _parameters.proportional_gain * 0.5f; // TODO [Max] REMOVE DIRTY HACK
                    // Should not be negative
                    current_allowed_velocity = current_allowed_velocity < 0.0f ? 0.0f : current_allowed_velocity;
                    if (fabsf(required_velocity) > current_allowed_velocity) {
                        velocity_vector *= current_allowed_velocity/fabsf(required_velocity); // TODO [Max] division by zero?
                    }
                }
                else if (dist_to_max_point < vehicle_dot_product && vehicle_dot_product < target_dot_product) {
                    // We are near last point and comming to it
                    current_allowed_velocity = (dist_to_max_point - 3.0f) * _parameters.proportional_gain * 0.5f; // TODO [Max] REMOVE DIRTY HACK
                    // Should not be negative
                    current_allowed_velocity = current_allowed_velocity < 0.0f ? 0.0f : current_allowed_velocity;
                    if (fabsf(required_velocity) > current_allowed_velocity) {
                        velocity_vector *= current_allowed_velocity/fabsf(required_velocity); // TODO [Max] division by zero?
                    }
                }

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
		}
	}
}
