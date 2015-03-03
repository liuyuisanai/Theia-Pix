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
 * @file loiter.cpp
 *
 * Helper class to loiter
 *
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 * @author Martins Frolovs <martins.f@airdog.vom>
 */

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>

#include <mavlink/mavlink_log.h>
#include <systemlib/err.h>

#include <uORB/uORB.h>
#include <uORB/topics/position_setpoint_triplet.h>

#include "loiter.h"
#include "navigator.h"

// Keep aligned with sub_mode enum!
const char* Loiter::mode_names[] = { "Landed",
		"Aim-and-shoot",
		"Look down",
		"Go-to-position",
		"Landing",
		"Taking-off"
};

Loiter::Loiter(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name),
	previous_target_valid_flag(false)
{
    updateParameters();
}

Loiter::~Loiter()
{
}

void
Loiter::on_inactive()
{
}

void
Loiter::on_activation()
{
	updateParameters();

	//Ignore all commands received from target so far
	update_vehicle_command();

    if (_parameters.first_point_lat == _parameters.last_point_lat
        && _parameters.first_point_lon == _parameters.last_point_lon
        && _parameters.first_point_lat != 0
        && _parameters.first_point_lon != 0
        ){
        // Deleting points, from logs and parameters, can't be valid
        int i_reset = 0;
        float f_reset = 0.0f;
        if (   param_set(param_find("NAV_CP_FIR_LA"), &i_reset)
            || param_set(param_find("NAV_CP_FIR_LO"), &i_reset)
            || param_set(param_find("NAV_CP_FIR_AL"), &f_reset)
            || param_set(param_find("NAV_CP_LAS_LA"), &i_reset)
            || param_set(param_find("NAV_CP_LAS_LO"), &i_reset)
            || param_set(param_find("NAV_CP_LAS_AL"), &f_reset)
           ) {
            mavlink_log_critical(_mavlink_fd, "ERROR: failed to save first leash point");
        } else {
            fprintf(stderr, "[loi] Eraised all params\n");
            //TODO [Max] send request to save parameters
        }
    } else {
        if (_parameters.first_point_lat != 0
                || _parameters.first_point_lon != 0
                || _parameters.first_point_alt != 0.0f) {
            double first_point[3];
            first_point[0] = _parameters.first_point_lat / 1e7;
            first_point[1] = _parameters.first_point_lon / 1e7;
            first_point[2] = _parameters.first_point_alt;
            _navigator->set_next_path_point(first_point, true, 0);
            fprintf(stderr, "[loi] setting first point\n");
        }
        if (_parameters.last_point_lat != 0
                || _parameters.last_point_lon != 0
                || _parameters.last_point_alt != 0.0f) {
            double last_point[3];
            last_point[0] = _parameters.last_point_lat / 1e7;
            last_point[1] = _parameters.last_point_lon / 1e7;
            last_point[2] = _parameters.last_point_alt;
            _navigator->set_next_path_point(last_point, true, 1);
            fprintf(stderr, "[loi] setting second point\n");
        }
        _navigator->publish_position_restriction();
    }

	// Determine current loiter sub mode
	struct vehicle_status_s *vstatus = _navigator->get_vstatus();

	// Prevent camera mode from resetting on activation
	previous_target_valid_flag = vstatus->condition_target_position_valid;

    _mavlink_fd = _navigator->get_mavlink_fd();

    // By default reset camera only if the camera mode has changed
    int8_t camera_reset_mode = 0;
    if (vstatus->nav_state_fallback) {
    	// Skip camera resetting in case we've fallbacked to Loiter
    	camera_reset_mode = -1;
    }

    _navigator->invalidate_setpoint_triplet();

    if (vstatus->auto_takeoff_cmd) {
		set_sub_mode(LOITER_SUB_MODE_TAKING_OFF, 1, camera_reset_mode);
		takeoff();
		if (vstatus->airdog_state == AIRD_STATE_IN_AIR || vstatus->airdog_state == AIRD_STATE_LANDING) {
			in_air_takeoff = true;
		}
		else {
			in_air_takeoff = false;
		}
		//resetModeArguments(MAIN_STATE_LOITER); //now done in commander itself

	} else if (vstatus->airdog_state == AIRD_STATE_LANDED || vstatus->airdog_state == AIRD_STATE_STANDBY) {
		set_sub_mode(LOITER_SUB_MODE_LANDED, 1, camera_reset_mode);
	} else {
		_camera_mode = UNDEFINED;
		set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 1, camera_reset_mode);
		commander_request_s *commander_request = _navigator->get_commander_request();
        commander_request->request_type = AIRD_STATE_CHANGE;
        commander_request->airdog_state = AIRD_STATE_IN_AIR;
        _navigator->set_commander_request_updated();
	}
}

void
Loiter::on_active()
{
	target_pos = _navigator->get_target_position();
	global_pos = _navigator->get_global_position();
	vehicle_status_s *vehicle_status = _navigator->get_vstatus();

	if (loiter_sub_mode == LOITER_SUB_MODE_TAKING_OFF && check_current_pos_sp_reached()) {

		if (in_air_takeoff) {
			// TODO! [AK] Consider resetting camera while "freeze" mode is not implemented, as currently there is no easy way to activate "aim" camera while freezed
			set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 2, -1);
		}
		else if (_parameters.airdog_init_pos_use == 1){
            set_sub_mode(LOITER_SUB_MODE_GO_TO_POSITION, 2);
            go_to_intial_position(); 
        }
        else {
        	if (_parameters.start_follow_immediately == 1) {
        		start_follow();
        	}
        	else {
        		set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 2);
        	}
        }
	}

	if (loiter_sub_mode == LOITER_SUB_MODE_LANDING && check_current_pos_sp_reached(SETPOINT_TYPE_LAND)) {
		set_sub_mode(LOITER_SUB_MODE_LANDED, 0);

		disarm();
	}

	if (loiter_sub_mode == LOITER_SUB_MODE_GO_TO_POSITION && check_current_pos_sp_reached()) {
		if (_parameters.start_follow_immediately == 1) {
        	start_follow();
    	}
    	else {
			set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 0);
		}
	}

	if (previous_target_valid_flag != vehicle_status->condition_target_position_valid) {
		if(vehicle_status->condition_target_position_valid) {
			// Refresh current submode if target signal regained and force camera reset
			set_sub_mode(loiter_sub_mode, 0, 1);
		}
		else {
			// Reset yaw only, 'cause we might be moving to initial position or something
			pos_sp_triplet = _navigator->get_position_setpoint_triplet();
			if (pos_sp_triplet->current.valid) {
				global_pos = _navigator->get_global_position();
				pos_sp_triplet->current.yaw = global_pos->yaw;
				pos_sp_triplet->current.yaw_valid = true;
				_navigator->set_position_setpoint_triplet_updated();
			}
		}
	}
	previous_target_valid_flag = vehicle_status->condition_target_position_valid;

	if ( update_vehicle_command() )
			execute_vehicle_command();
	
}

void
Loiter::execute_vehicle_command()
{
    updateParams();

	vehicle_command_s cmd = _vcommand;

	switch (loiter_sub_mode){
		case LOITER_SUB_MODE_LANDED:
			execute_command_in_landed(cmd);
			break;
		case LOITER_SUB_MODE_AIM_AND_SHOOT:
			execute_command_in_aim_and_shoot(cmd);
			break;
		case LOITER_SUB_MODE_LOOK_DOWN:
			execute_command_in_look_down(cmd);
			break;
		case LOITER_SUB_MODE_GO_TO_POSITION:
			execute_command_in_go_to_position(cmd);
			break;
		case LOITER_SUB_MODE_LANDING:
			execute_command_in_landing(cmd);
			break;
		case LOITER_SUB_MODE_TAKING_OFF:
			execute_command_in_taking_off(cmd);
			break;
	}

}

void
Loiter::execute_command_in_landed(vehicle_command_s cmd){

	// TODO! [AK] Is this thing still needed? Or do we switch to takeoff parameter in all cases?
	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {

		int remote_cmd = cmd.param1;

		if (remote_cmd == REMOTE_CMD_TAKEOFF) {
			set_sub_mode(LOITER_SUB_MODE_TAKING_OFF, 0);
			takeoff();
		} else if (remote_cmd == REMOTE_CMD_LAND_DISARM) {
			disarm();
		}
	}
}

void
Loiter::execute_command_in_aim_and_shoot(vehicle_command_s cmd){



	// Calculate offset
	float offset_x;
	float offset_y;
	float offset_z = target_pos->alt - global_pos->alt;

	get_vector_to_next_waypoint(
			target_pos->lat,
			target_pos->lon,
			global_pos->lat,
			global_pos->lon,
			&offset_x,
			&offset_y
	);

	math::Vector<3> offset(offset_x, offset_y, offset_z);

	vehicle_status_s *vehicle_status = _navigator->get_vstatus();

	if (cmd.command == VEHICLE_CMD_DO_SET_MODE){

		//uint8_t base_mode = (uint8_t)cmd.param1;
		uint8_t main_mode = (uint8_t)cmd.param2;

		if (main_mode == PX4_CUSTOM_MAIN_MODE_RTL) {

			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_RTL;
			_navigator->set_commander_request_updated();

		}
	}


	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {

		REMOTE_CMD remote_cmd = (REMOTE_CMD)cmd.param1;

		pos_sp_triplet = _navigator->get_position_setpoint_triplet();

		pos_sp_triplet->previous.valid = false;
		pos_sp_triplet->current.valid = true;
		pos_sp_triplet->next.valid = false;

		switch(remote_cmd) {
			case  REMOTE_CMD_LAND_DISARM: {

				// Switch main state in case land command was received to prevent unintended land interrupts
				if (vehicle_status->nav_state_fallback && vehicle_status->main_state != MAIN_STATE_LOITER) {
					commander_request_s *commander_request = _navigator->get_commander_request();
					commander_request->request_type = V_MAIN_STATE_CHANGE;
					commander_request->main_state = MAIN_STATE_EMERGENCY_LAND;
					_navigator->set_commander_request_updated();
				}
				else {
					mavlink_log_info(_navigator->get_mavlink_fd(), "Land disarm command");
					set_sub_mode(LOITER_SUB_MODE_LANDING, 0);
					land();
				}
				break;
			}
            case REMOTE_CMD_GOTO_DEFUALT_DST: {
                go_to_intial_position();
                break;
            }
			case REMOTE_CMD_UP: {

				pos_sp_triplet->current.alt = global_pos->alt + _parameters.up_button_step;
				pos_sp_triplet->current.lat = global_pos->lat;
				pos_sp_triplet->current.lon = global_pos->lon;
				pos_sp_triplet->current.position_valid = true;

				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;

				break;
			}
			case REMOTE_CMD_DOWN: {

				pos_sp_triplet->current.alt = global_pos->alt - _parameters.down_button_step;
				pos_sp_triplet->current.lat = global_pos->lat;
				pos_sp_triplet->current.lon = global_pos->lon;
				pos_sp_triplet->current.position_valid = true;

				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
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
				double alpha = (double)_parameters.horizon_button_step / radius;

				// vector yaw rotation +alpha or -alpha depending on left or right
				R_phi.from_euler(0.0f, 0.0f, -alpha);
				math::Vector<3> offset_new  = R_phi * offset;

				double lat_new, lon_new;
				add_vector_to_global_position(
						(*target_pos).lat,
						(*target_pos).lon,
						offset_new(0),
						offset_new(1),
						&lat_new,
						&lon_new
				);

				pos_sp_triplet->current.lat = lat_new;
				pos_sp_triplet->current.lon = lon_new;
				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->current.position_valid = true;

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
				double alpha = (double)_parameters.horizon_button_step / radius;

				// vector yaw rotation +alpha or -alpha depending on left or right
				R_phi.from_euler(0.0f, 0.0f, +alpha);
				math::Vector<3> offset_new  = R_phi * offset;

				double lat_new, lon_new;
				add_vector_to_global_position(
						(*target_pos).lat,
						(*target_pos).lon,
						offset_new(0),
						offset_new(1),
						&lat_new,
						&lon_new
				);

				pos_sp_triplet->current.lat = lat_new;
				pos_sp_triplet->current.lon = lon_new;
				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->current.position_valid = true;

				break;
			}
			case REMOTE_CMD_CLOSER: {

				// Calculate vector angle from target to device with atan2(y, x)
				float alpha = atan2f(offset(1), offset(0));

				// Create vector in the same direction, with loiter_step length
				math::Vector<3> offset_delta(
						cosf(alpha) * _parameters.horizon_button_step,
						sinf(alpha) * _parameters.horizon_button_step,
						0);

				math::Vector<3> offset_new = offset - offset_delta;

				double lat_new, lon_new;
				add_vector_to_global_position(
						(*target_pos).lat,
						(*target_pos).lon,
						offset_new(0),
						offset_new(1),
						&lat_new,
						&lon_new
				);

				pos_sp_triplet->current.lat = lat_new;
				pos_sp_triplet->current.lon = lon_new;
				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->current.position_valid = true;

				break;

			}

			case REMOTE_CMD_FURTHER: {

				// Calculate vector angle from target to device with atan2(y, x)
				float alpha = atan2(offset(1), offset(0));

				// Create vector in the same direction, with loiter_step length
				math::Vector<3> offset_delta(
						cosf(alpha) * _parameters.horizon_button_step,
						sinf(alpha) * _parameters.horizon_button_step,
						0);

				math::Vector<3> offset_new = offset + offset_delta;

				double lat_new, lon_new;
				add_vector_to_global_position(
						target_pos->lat,
						target_pos->lon,
						offset_new(0),
						offset_new(1),
						&lat_new,
						&lon_new
				);

				pos_sp_triplet->current.lat = lat_new;
				pos_sp_triplet->current.lon = lon_new;
				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->current.position_valid = true;

				break;
			}
			case REMOTE_CMD_COME_TO_ME: {

				pos_sp_triplet->current.lat = target_pos->lat;
				pos_sp_triplet->current.lon = target_pos->lon;
				pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->current.position_valid = true;

				set_sub_mode(LOITER_SUB_MODE_GO_TO_POSITION, 0);

				break;
			}
			case REMOTE_CMD_LOOK_DOWN: {

				set_sub_mode(LOITER_SUB_MODE_LOOK_DOWN, 0);
				break;

			}
			case REMOTE_CMD_PLAY_PAUSE: {
                start_follow();
				break;
			}
            case REMOTE_CMD_SET_FIRST_POINT: {
                    double point[3] = {
                          global_pos->lat
                        , global_pos->lon
                        , global_pos->alt
                    };
                    _navigator->set_next_path_point(point, true, 0);
                    _navigator->publish_position_restriction();
                    break;
            }
            case REMOTE_CMD_SET_SECOND_POINT: {
                    double point[3] = {
                          global_pos->lat
                        , global_pos->lon
                        , global_pos->alt
                    };
                    _navigator->set_next_path_point(point, true, 1);
                    _navigator->publish_position_restriction();
                    break;
            }
            case REMOTE_CMD_CLEAR_POINTS: {
                    _navigator->clear_path_points();
                    _navigator->publish_position_restriction();
                    break;
            }

		}

		_navigator->set_position_setpoint_triplet_updated();

	}

}

void
Loiter::execute_command_in_look_down(vehicle_command_s cmd){

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		int remote_cmd = cmd.param1;
		if (remote_cmd == REMOTE_CMD_PLAY_PAUSE) {
			set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 1);
		}
	}
}

void
Loiter::execute_command_in_go_to_position(vehicle_command_s cmd){

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		int remote_cmd = cmd.param1;
		if (remote_cmd == REMOTE_CMD_PLAY_PAUSE) {
			set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 1);
		}
	}

}

void
Loiter::execute_command_in_landing(vehicle_command_s cmd){

	// TODO! [AK] Correct pausing
	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		int remote_cmd = cmd.param1;
		if (remote_cmd == REMOTE_CMD_PLAY_PAUSE) {
			// Update airdog state
			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = AIRD_STATE_CHANGE;
			commander_request->airdog_state = AIRD_STATE_IN_AIR;
			_navigator->set_commander_request_updated();

			_navigator->invalidate_setpoint_triplet();
			set_sub_mode(LOITER_SUB_MODE_AIM_AND_SHOOT, 1);
			loiter_sub_mode = LOITER_SUB_MODE_AIM_AND_SHOOT;
		}
	}
}

/**
 * @param reset_setpoint: 0 = false; 1 = reset position and type; 2 = reset type only
 * @param force_camera_reset: -1 => skip camera change; 0 => change only if state changed; 1 => force reset
 */
void
Loiter::set_sub_mode(LOITER_SUB_MODE new_sub_mode, uint8_t reset_setpoint, int8_t force_camera_reset) {

	if (reset_setpoint > 0) {
		pos_sp_triplet = _navigator->get_position_setpoint_triplet();

		pos_sp_triplet->previous.valid = false;
		pos_sp_triplet->current.valid = true;
		pos_sp_triplet->next.valid = false;

		pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
		
		if (reset_setpoint == 1) {
			// Reset setpoint position to current global position
			global_pos = _navigator->get_global_position();
			
			pos_sp_triplet->current.alt = global_pos->alt;
			pos_sp_triplet->current.lon = global_pos->lon;
			pos_sp_triplet->current.lat = global_pos->lat;

			pos_sp_triplet->current.yaw = global_pos->yaw;

			pos_sp_triplet->current.position_valid = true;
			pos_sp_triplet->current.yaw_valid = true;
		}

		_navigator->set_position_setpoint_triplet_updated();
	}

	loiter_sub_mode = new_sub_mode;

	if (force_camera_reset != -1) {
		switch(new_sub_mode){
			case LOITER_SUB_MODE_AIM_AND_SHOOT:
				set_camera_mode(AIM_TO_TARGET, force_camera_reset == 1);
				break;
			case LOITER_SUB_MODE_LOOK_DOWN:
				set_camera_mode(LOOK_DOWN, force_camera_reset == 1);
				break;
			case LOITER_SUB_MODE_GO_TO_POSITION:
				set_camera_mode(AIM_TO_TARGET, force_camera_reset == 1);
				break;
			case LOITER_SUB_MODE_LANDING:
				set_camera_mode(HORIZONTAL, force_camera_reset == 1);
				break;
			case LOITER_SUB_MODE_TAKING_OFF:
				set_camera_mode(HORIZONTAL, force_camera_reset == 1);
				break;
			case LOITER_SUB_MODE_LANDED:
				break;
		}
	}

	mavlink_log_info(_mavlink_fd, "[loiter] Loiter sub mode set to %s ! ", Loiter::mode_names[new_sub_mode]);

}

void
Loiter::execute_command_in_taking_off(vehicle_command_s cmd) {
}

void
Loiter::start_follow() {
	if (_parameters.afol_mode == 0) {

    	commander_request_s *commander_request = _navigator->get_commander_request();
		commander_request->request_type = V_MAIN_STATE_CHANGE;
		commander_request->main_state = MAIN_STATE_ABS_FOLLOW;
		_navigator->set_commander_request_updated();

    } else if (_parameters.afol_mode == 1) {
         
        commander_request_s *commander_request = _navigator->get_commander_request();
        commander_request->request_type = V_MAIN_STATE_CHANGE;
        commander_request->main_state = MAIN_STATE_AUTO_PATH_FOLLOW;

        _navigator->set_flag_reset_pfol_offs(true);

        _navigator->set_commander_request_updated();
    
    } else if (_parameters.afol_mode == 2) {
        commander_request_s *commander_request = _navigator->get_commander_request();
        commander_request->request_type = V_MAIN_STATE_CHANGE;
        commander_request->main_state = MAIN_STATE_CABLE_PARK;
        _navigator->set_commander_request_updated();
    }
}
