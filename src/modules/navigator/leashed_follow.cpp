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
}

void
Leashed::on_active()
{
	// Execute command if received
	if ( update_vehicle_command() )
			execute_vehicle_command();
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
            case REMOTE_CMD_UP: {
                double first[3];
                //double last[3];
                _navigator->get_path_points(0, first);
                first[2] += (double)_parameters.loi_step_len;
                _navigator->set_next_path_point(first, true, 0);
                _navigator->publish_position_restriction();
                break;
            }
            case REMOTE_CMD_DOWN: {
                double first[3];
                _navigator->get_path_points(0, first);
                first[2] -= (double)_parameters.loi_step_len;
                _navigator->set_next_path_point(first, true, 0);
                _navigator->publish_position_restriction();
                break;
            }
		}
	}
}
