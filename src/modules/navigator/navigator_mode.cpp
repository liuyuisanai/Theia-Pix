/****************************************************************************
 *
 *   Copyright (c) 2014 PX4 Development Team. All rights reserved.
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
 * @file navigator_mode.cpp
 *
 * Base class for different modes in navigator
 *
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#include "navigator_mode.h"
#include "navigator.h"

#include <uORB/uORB.h>
#include <uORB/topics/position_setpoint_triplet.h>
#include <uORB/topics/parameter_update.h>

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <drivers/device/device.h>
#include <drivers/drv_hrt.h>
#include <arch/board/board.h>

#include <systemlib/err.h>
#include <systemlib/systemlib.h>
#include <geo/geo.h>
#include <dataman/dataman.h>
#include <mathlib/mathlib.h>
#include <mavlink/mavlink_log.h>


NavigatorMode::NavigatorMode(Navigator *navigator, const char *name) :
	SuperBlock(navigator, name),
	_navigator(navigator),
	_first_run(true)
{
	updateParams();
	on_inactive();
	_mavlink_fd = open(MAVLINK_LOG_DEVICE, 0);
}

NavigatorMode::~NavigatorMode()
{
}

void
NavigatorMode::updateParameters() {

	updateParamHandles();
	updateParamValues();
}

void
NavigatorMode::updateParamHandles() {

    _parameter_handles.first_point_lat = param_find("NAV_CP_FIR_LA");
    _parameter_handles.first_point_lon = param_find("NAV_CP_FIR_LO");
    _parameter_handles.first_point_alt = param_find("NAV_CP_FIR_AL");
    _parameter_handles.last_point_lat = param_find("NAV_CP_LAS_LA");
    _parameter_handles.last_point_lon = param_find("NAV_CP_LAS_LO");
    _parameter_handles.last_point_alt = param_find("NAV_CP_LAS_AL");

    _parameter_handles.down_button_step = param_find("NAV_DOWN_STEP");
    _parameter_handles.up_button_step = param_find("NAV_UP_STEP");
    _parameter_handles.horizon_button_step = param_find("NAV_HOR_STEP");
	_parameter_handles.takeoff_alt = param_find("NAV_TAKEOFF_ALT");
	_parameter_handles.takeoff_acceptance_radius = param_find("NAV_TAKEOFF_ACR");
	_parameter_handles.acceptance_radius = param_find("NAV_ACC_RAD");

	_parameter_handles.afol_mode = param_find("NAV_AFOL_MODE");

	_parameter_handles.rtl_ret_alt = param_find("RTL_RET_ALT");

	_parameter_handles.pafol_buf_size = param_find("PAFOL_BUFF_SIZE");
	_parameter_handles.pafol_optimal_dist = param_find("PAFOL_OPT_D");
	_parameter_handles.pafol_break_dist = param_find("PAFOL_BRK_D");
	_parameter_handles.pafol_break_coef = param_find("PAFOL_BRK_C");
	_parameter_handles.pafol_min_alt_off = param_find("PAFOL_ALT_OFF");

    _parameter_handles.pafol_acc_dst_to_line = param_find("PAFOL_AC_DST_LN");
    _parameter_handles.pafol_acc_dst_to_point = param_find("PAFOL_AC_DST_PT");
    _parameter_handles.pafol_acc_rad = param_find("PAFOL_ACC_RAD");

    _parameter_handles.pafol_vel_i = param_find("PAFOL_VPID_I");
    _parameter_handles.pafol_vel_p = param_find("PAFOL_VPID_P");
    _parameter_handles.pafol_vel_d = param_find("PAFOL_VPID_D");
    
    _parameter_handles.pafol_vel_i_add_dec_rate = param_find("PAFOL_VPID_I_DR");
    _parameter_handles.pafol_vel_i_add_inc_rate = param_find("PAFOL_VPID_I_IR");
    _parameter_handles.pafol_vel_i_upper_limit = param_find("PAFOL_VPID_I_UL");
    _parameter_handles.pafol_vel_i_lower_limit = param_find("PAFOL_VPID_I_LL");



	_parameter_handles.mpc_max_speed = param_find("MPC_XY_VEL_MAX");
    _parameter_handles.airdog_dst_inv = param_find("A_DST_INV");
    _parameter_handles.airdog_init_pos_dst = param_find("A_INIT_POS_D");
    _parameter_handles.airdog_init_pos_use = param_find("A_INIT_POS_U");

    _parameter_handles.a_yaw_ignore_radius = param_find("A_YAW_IGNR_R");
    _parameter_handles.proportional_gain = param_find("MPC_XY_P");
	_parameter_handles.follow_rpt_alt	= param_find("FOL_RPT_ALT");
	_parameter_handles.start_follow_immediately = param_find("A_FOL_IMDTLY");
}

void
NavigatorMode::updateParamValues() {

    param_get(_parameter_handles.first_point_lat, &(_parameters.first_point_lat));
    param_get(_parameter_handles.first_point_lon, &(_parameters.first_point_lon));
    param_get(_parameter_handles.first_point_alt, &(_parameters.first_point_alt));
    param_get(_parameter_handles.last_point_lat, &(_parameters.last_point_lat));
    param_get(_parameter_handles.last_point_lon, &(_parameters.last_point_lon));
    param_get(_parameter_handles.last_point_alt, &(_parameters.last_point_alt));
    param_get(_parameter_handles.down_button_step, &(_parameters.down_button_step));
    param_get(_parameter_handles.up_button_step, &(_parameters.up_button_step));
    param_get(_parameter_handles.horizon_button_step, &(_parameters.horizon_button_step));
	param_get(_parameter_handles.takeoff_alt, &(_parameters.takeoff_alt));
	param_get(_parameter_handles.takeoff_acceptance_radius, &(_parameters.takeoff_acceptance_radius));
	param_get(_parameter_handles.acceptance_radius, &(_parameters.acceptance_radius));
	param_get(_parameter_handles.afol_mode, &(_parameters.afol_mode));

	param_get(_parameter_handles.rtl_ret_alt, &(_parameters.rtl_ret_alt));

	param_get(_parameter_handles.pafol_buf_size, &(_parameters.pafol_buf_size));
	param_get(_parameter_handles.pafol_optimal_dist, &(_parameters.pafol_optimal_dist));
	param_get(_parameter_handles.pafol_break_dist, &(_parameters.pafol_break_dist));
	param_get(_parameter_handles.pafol_break_coef, &(_parameters.pafol_break_coef));
	param_get(_parameter_handles.pafol_min_alt_off, &(_parameters.pafol_min_alt_off));
	param_get(_parameter_handles.pafol_acc_rad, &(_parameters.pafol_acc_rad));
	param_get(_parameter_handles.pafol_acc_dst_to_line, &(_parameters.pafol_acc_dst_to_line));
	param_get(_parameter_handles.pafol_acc_dst_to_point, &(_parameters.pafol_acc_dst_to_point));


	param_get(_parameter_handles.pafol_vel_i_add_dec_rate, &(_parameters.pafol_vel_i_add_dec_rate));
	param_get(_parameter_handles.pafol_vel_i_add_inc_rate, &(_parameters.pafol_vel_i_add_inc_rate));
	param_get(_parameter_handles.pafol_vel_i_upper_limit, &(_parameters.pafol_vel_i_upper_limit));
	param_get(_parameter_handles.pafol_vel_i_lower_limit, &(_parameters.pafol_vel_i_lower_limit));

	param_get(_parameter_handles.pafol_vel_i, &(_parameters.pafol_vel_i));
	param_get(_parameter_handles.pafol_vel_p, &(_parameters.pafol_vel_p));
	param_get(_parameter_handles.pafol_vel_d, &(_parameters.pafol_vel_d));

	param_get(_parameter_handles.mpc_max_speed, &(_parameters.mpc_max_speed));
	param_get(_parameter_handles.rtl_ret_alt, &(_parameters.rtl_ret_alt));

	param_get(_parameter_handles.airdog_dst_inv, &(_parameters.airdog_dst_inv));
	param_get(_parameter_handles.airdog_init_pos_dst, &(_parameters.airdog_init_pos_dst));
	param_get(_parameter_handles.airdog_init_pos_use, &(_parameters.airdog_init_pos_use));

    param_get(_parameter_handles.a_yaw_ignore_radius, &(_parameters.a_yaw_ignore_radius));
    param_get(_parameter_handles.proportional_gain, &(_parameters.proportional_gain));
    param_get(_parameter_handles.follow_rpt_alt, &(_parameters.follow_rpt_alt));
    param_get(_parameter_handles.start_follow_immediately, &(_parameters.start_follow_immediately));
    
}


void
NavigatorMode::run(bool active, bool parameters_updated) {

    if (parameters_updated) {
        updateParameters();    
    }

	if (active) {
		if (_first_run) {
			/* first run */
			_first_run = false;
			/* Reset stay in failsafe flag */
			_navigator->get_mission_result()->stay_in_failsafe = false;
			_navigator->publish_mission_result();
			on_activation();

		} else {
			/* periodic updates when active */
			on_active();
		}

	} else {
		/* periodic updates when inactive */
		_first_run = true;
		on_inactive();
	}
}

void
NavigatorMode::on_inactive()
{
}

void
NavigatorMode::on_activation()
{
	_navigator->invalidate_setpoint_triplet();
}

void
NavigatorMode::on_active()
{
}

bool
NavigatorMode::update_vehicle_command()
{
	bool vcommand_updated = false;
	orb_check(_navigator->get_vehicle_command_sub(), &vcommand_updated);

	if (vcommand_updated) {

		if (orb_copy(ORB_ID(vehicle_command), _navigator->get_vehicle_command_sub(), &_vcommand) == OK) {
			return true;
		}
		else
			return false;
	}

	return false;
}

void
NavigatorMode::execute_vehicle_command()
{
}

void
NavigatorMode::set_camera_mode(camera_mode_t camera_mode, bool force_change)
{

	if (_camera_mode != camera_mode || force_change) {

		_camera_mode = camera_mode;

		commander_request_s *commander_request = _navigator->get_commander_request();
		commander_request->camera_mode_changed = true;
		commander_request->camera_mode = camera_mode;
		_navigator->set_commander_request_updated();

		pos_sp_triplet = _navigator->get_position_setpoint_triplet();

		if (camera_mode != AIM_TO_TARGET) {
			global_pos = _navigator->get_global_position();
			//keep current yaw angle;
			pos_sp_triplet->current.yaw = global_pos->yaw;
			pos_sp_triplet->current.yaw_valid = true;
			switch (camera_mode) {
				case HORIZONTAL :
					pos_sp_triplet->current.camera_pitch = 0.0f;		
					break;
				case LOOK_DOWN :
					pos_sp_triplet->current.camera_pitch = -1.0f;
					break;
			}
			pos_sp_triplet->current.camera_pitch_valid = true;

		}
		else {
			pos_sp_triplet->current.camera_pitch_valid = false;
		}
		_navigator->set_position_setpoint_triplet_updated();
	}
}

bool
NavigatorMode::check_current_pos_sp_reached(SETPOINT_TYPE expected_sp_type)
{	
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	if (expected_sp_type != SETPOINT_TYPE_UNDEFINED && pos_sp_triplet->current.type != expected_sp_type) {
		return false;
	}

	struct vehicle_status_s *vstatus = _navigator->get_vstatus();	
	global_pos = _navigator->get_global_position();;
	
	switch (pos_sp_triplet->current.type)
	{
	case SETPOINT_TYPE_IDLE:
		return true;
		break;

	case SETPOINT_TYPE_LAND:

        if (vstatus->condition_landed){

        	// TODO! [AK] Is not sent in most cases, as "disarm" request in the same cycle overrides it
            commander_request_s *commander_request = _navigator->get_commander_request();
            commander_request->request_type = AIRD_STATE_CHANGE;
            commander_request->airdog_state = AIRD_STATE_LANDED;
            _navigator->set_commander_request_updated();
            return true;

        }

        return false;

		break;

	case SETPOINT_TYPE_TAKEOFF:
	{
		float alt_diff = fabs(pos_sp_triplet->current.alt - global_pos->alt);

        if (_parameters.takeoff_acceptance_radius >= alt_diff) {

            commander_request_s *commander_request = _navigator->get_commander_request();
            commander_request->request_type = AIRD_STATE_CHANGE;
            commander_request->airdog_state = AIRD_STATE_IN_AIR;
            _navigator->set_commander_request_updated();

            return true;

        }

        return false;
        
		break;
	}
	case SETPOINT_TYPE_POSITION:
	{
		float dist_xy = -1;
		float dist_z = -1;

		float distance = get_distance_to_point_global_wgs84(
			global_pos->lat, global_pos->lon, global_pos->alt,
			pos_sp_triplet->current.lat, pos_sp_triplet->current.lon, pos_sp_triplet->current.alt,
			&dist_xy, &dist_z
		);

        if (_parameters.acceptance_radius >= distance){
            return true; 
        }

        return false;

		break;
	}
	default:
		return false;
		break;

	}
}


void
NavigatorMode::land(uint8_t reset_setpoint)
{
    pos_sp_triplet = _navigator->get_position_setpoint_triplet();
    global_pos = _navigator->get_global_position();

	if (reset_setpoint == 1) {
		// Will update local pos_sp_triplet as it is a pointer to the same struct
		_navigator->invalidate_setpoint_triplet();

		pos_sp_triplet->current.valid = true;

		pos_sp_triplet->current.lat = global_pos->lat;
		pos_sp_triplet->current.lon = global_pos->lon;
		pos_sp_triplet->current.alt = global_pos->alt;
		pos_sp_triplet->current.position_valid = true;
		pos_sp_triplet->current.yaw = NAN;
		pos_sp_triplet->current.yaw_valid = false;
	}

	pos_sp_triplet->current.type = SETPOINT_TYPE_LAND;

	_navigator->set_position_setpoint_triplet_updated();

	commander_request_s *commander_request = _navigator->get_commander_request();
	commander_request->request_type = AIRD_STATE_CHANGE;
    commander_request->airdog_state = AIRD_STATE_LANDING;
	_navigator->set_commander_request_updated();
}

void
NavigatorMode::takeoff()
{
	_navigator->invalidate_setpoint_triplet();
    pos_sp_triplet = _navigator->get_position_setpoint_triplet();
    global_pos = _navigator->get_global_position();
    _vstatus = _navigator->get_vstatus();

	pos_sp_triplet->previous.valid = false;
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->next.valid = false;

	pos_sp_triplet->current.lat = global_pos->lat;
	pos_sp_triplet->current.lon = global_pos->lon;

	if (_vstatus->airdog_state == AIRD_STATE_IN_AIR) {
		// Complete takeoff procedure immediately if IN AIR
		pos_sp_triplet->current.alt = global_pos->alt;
	}
	else if (_vstatus->airdog_state == AIRD_STATE_LANDING) {
		// Check range finder and try to calculate necessary altitude gain
		// TODO! [AK] Use parameter instead of hardcoded 3.0f
		pos_sp_triplet->current.alt = global_pos->alt + 3.0f; // - range_finder.distance
	}
	else {
		pos_sp_triplet->current.alt = global_pos->alt + _parameters.takeoff_alt;
	}

	pos_sp_triplet->current.position_valid = true;

	pos_sp_triplet->current.yaw = global_pos->yaw;//NAN;
	pos_sp_triplet->current.yaw_valid = true;
	pos_sp_triplet->current.type = SETPOINT_TYPE_TAKEOFF;

	_navigator->set_position_setpoint_triplet_updated();

	commander_request_s *commander_request = _navigator->get_commander_request();
	commander_request->request_type = AIRD_STATE_CHANGE;
    commander_request->airdog_state = AIRD_STATE_TAKING_OFF;
	_navigator->set_commander_request_updated();

}


void
NavigatorMode::disarm()
{
	_navigator->invalidate_setpoint_triplet();
	commander_request_s *commander_request = _navigator->get_commander_request();
	commander_request->request_type = V_DISARM;
	_navigator->set_commander_request_updated();
}
void
NavigatorMode::resetModeArguments(main_state_t main_state)
{
	commander_request_s *commander_request = _navigator->get_commander_request();
	commander_request->request_type = V_RESET_MODE_ARGS;
	commander_request->main_state = main_state;
	_navigator->set_commander_request_updated();
}

void
NavigatorMode::go_to_intial_position(){

            global_pos = _navigator->get_global_position();
            target_pos = _navigator->get_target_position();

            float offset_x;
            float offset_y;
            float offset_z = global_pos->alt - target_pos->alt;

            get_vector_to_next_waypoint(
                    target_pos->lat,
                    target_pos->lon,
                    global_pos->lat,
                    global_pos->lon,
                    &offset_x,
                    &offset_y
            );

            float alpha = atan2(offset_y, offset_x);

            math::Vector<3> new_drone_offset(
                cosf(alpha) * _parameters.airdog_init_pos_dst,
                sinf(alpha) * _parameters.airdog_init_pos_dst,
                offset_z
                );

            double lat_new;
            double lon_new;
            //double alt_new = target_pos->alt + _parameters.takeoff_alt;
            
            add_vector_to_global_position(
                    target_pos->lat,
                    target_pos->lon,
                    new_drone_offset(0),
                    new_drone_offset(1),
                    &lat_new,
                    &lon_new
            );
            
            float dst = sqrt( offset_x * offset_x + offset_y * offset_y);

            if (dst <= _parameters.airdog_dst_inv && dst > _parameters.airdog_init_pos_dst) {
                _navigator->invalidate_setpoint_triplet();

                pos_sp_triplet->current.yaw = _wrap_pi(atan2f(-new_drone_offset(1), -new_drone_offset(0)));
                pos_sp_triplet->current.yaw_valid = true;

                pos_sp_triplet->current.valid = true;
                pos_sp_triplet->previous.valid = false;
                pos_sp_triplet->next.valid = false;

                pos_sp_triplet->current.lat = lat_new;
                pos_sp_triplet->current.lon = lon_new;
                //pos_sp_triplet->current.alt = alt_new;
                pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
                pos_sp_triplet->current.position_valid = true;
                
                _navigator->set_position_setpoint_triplet_updated();
            }
}
