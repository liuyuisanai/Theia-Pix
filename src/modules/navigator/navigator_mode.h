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
 * @file navigator_mode.h
 *
 * Base class for different modes in navigator
 *
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 */

#ifndef NAVIGATOR_MODE_H
#define NAVIGATOR_MODE_H

#include <drivers/drv_hrt.h>

#include <controllib/blocks.hpp>
#include <controllib/block/BlockParam.hpp>

#include <dataman/dataman.h>

#include <uORB/topics/position_setpoint_triplet.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/vehicle_status.h>
 #include <uORB/topics/commander_request.h>

class Navigator;

class NavigatorMode : public control::SuperBlock
{
public:
	/**
	 * Constructor
	 */
	NavigatorMode(Navigator *navigator, const char *name);

	/**
	 * Destructor
	 */
	virtual ~NavigatorMode();

	void run(bool active, bool parameters_updated);

	/**
	 * This function is called while the mode is inactive
	 */
	virtual void on_inactive();

	/**
	 * This function is called one time when mode become active, poos_sp_triplet must be initialized here
	 */
	virtual void on_activation();

	/**
	 * This function is called while the mode is active
	 */
	virtual void on_active();

	/*
	 * This function defines how vehicle reacts on commands in
	 * current navigator mode
	 */
	virtual void execute_vehicle_command();

	/*
	 * Check if vehicle command subscription has been updated
	 * if it has it updates _vcommand structure and returns true
	 */
	bool update_vehicle_command();

	void set_camera_mode(camera_mode_t camera_mode, bool force_change = false);

	inline void updateParameters();
	void updateParamValues();
	void updateParamHandles();

    void land(uint8_t reset_setpoint = 1);
    void takeoff();
    void disarm();
    void resetModeArguments(main_state_t main_state);


	struct {
        int first_point_lat;
        int first_point_lon;
        float first_point_alt;
        int last_point_lat;
        int last_point_lon;
        float last_point_alt;
        float down_button_step;
        float up_button_step;
        float horizon_button_step;
		float takeoff_alt;
		float takeoff_acceptance_radius;
		float acceptance_radius;

        int afol_mode;

		float rtl_ret_alt;

		ssize_t pafol_buf_size;
		float pafol_break_dist;
		float pafol_break_coef;
		float pafol_optimal_dist;
		float pafol_min_alt_off;
        float pafol_acc_rad;
        float pafol_acc_dst_to_gate;
        float pafol_gate_width;


        float pafol_vel_i;
        float pafol_vel_p;
        float pafol_vel_d;

        float pafol_vel_i_add_dec_rate;
        float pafol_vel_i_add_inc_rate;

        float pafol_vel_i_lower_limit;
        float pafol_vel_i_upper_limit;

        float pafol_backward_distance_limit;

		float mpc_max_speed;

        float airdog_dst_inv; 
        float airdog_init_pos_dst;
        int airdog_init_pos_use; 

        int follow_rpt_alt;

        float a_yaw_ignore_radius;
        // Proportional gain for horizontal position error
        float proportional_gain;

        int start_follow_immediately;

        float airdog_traj_radius;

	} _parameters;		


	struct {
        param_t first_point_lat;
        param_t first_point_lon;
        param_t first_point_alt;
        param_t last_point_lat;
        param_t last_point_lon;
        param_t last_point_alt;
        param_t down_button_step;
        param_t up_button_step;
        param_t horizon_button_step;
		param_t takeoff_alt;
		param_t takeoff_acceptance_radius;
		param_t acceptance_radius;

        param_t afol_mode;

		param_t rtl_ret_alt;

		param_t pafol_buf_size;
		param_t pafol_break_coef;
		param_t pafol_break_dist;
		param_t pafol_optimal_dist;
		param_t pafol_min_alt_off;
        param_t pafol_acc_rad;

        param_t pafol_acc_dst_to_gate;
        param_t pafol_gate_width;

        param_t pafol_acc_dst_to_line;
        param_t pafol_acc_dst_to_point;
        param_t pafol_stop_speed;

        param_t pafol_vel_i;
        param_t pafol_vel_p;
        param_t pafol_vel_d;

        param_t pafol_vel_i_add_dec_rate;
        param_t pafol_vel_i_add_inc_rate;

        param_t pafol_vel_i_lower_limit;
        param_t pafol_vel_i_upper_limit;

        param_t pafol_backward_distance_limit;

		param_t mpc_max_speed;

        param_t airdog_dst_inv;
        param_t airdog_init_pos_dst; 
        param_t airdog_init_pos_use;

        param_t follow_rpt_alt;

        param_t a_yaw_ignore_radius;
        param_t proportional_gain;
        param_t start_follow_immediately;

        param_t airdog_traj_radius;

	} _parameter_handles;



protected:
	Navigator *_navigator;

	struct position_setpoint_triplet_s 	*pos_sp_triplet;

	struct target_global_position_s 	*target_pos;
	struct vehicle_global_position_s 	*global_pos;
	struct home_position_s 				*home_pos;
	struct vehicle_status_s				*_vstatus;

	struct vehicle_command_s _vcommand;

	int		_mavlink_fd;			/**< the file descriptor to send messages over mavlink */


	bool check_current_pos_sp_reached(SETPOINT_TYPE expected_sp_type = SETPOINT_TYPE_UNDEFINED);
    void go_to_intial_position();
    camera_mode_t _camera_mode;

private:

	bool _first_run;

	/*
	 * This class has ptr data members, so it should not be copied,
	 * consequently the copy constructors are private.
	 */
	NavigatorMode(const NavigatorMode&);
	NavigatorMode operator=(const NavigatorMode&);

};

#endif
