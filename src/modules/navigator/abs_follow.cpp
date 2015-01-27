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
	_navigator->invalidate_setpoint_triplet();
}

void
AbsFollow::on_active()
{
	// Execute command if received
	if ( update_vehicle_command() )
			execute_vehicle_command();
}

void
AbsFollow::execute_vehicle_command() {
    // Update _parameter values with the latest navigator_mode parameters
    memcpy(&_parameters, &(NavigatorMode::_parameters), sizeof(_parameters));
	vehicle_command_s cmd = _vcommand;
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
		math::Vector<3> offset =_afollow_offset;
		switch(remote_cmd){
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}
			case  REMOTE_CMD_LAND_DISARM: {
                commander_request_s *commander_request = _navigator->get_commander_request();
                commander_request->request_type = V_MAIN_STATE_CHANGE;
                commander_request->main_state = MAIN_STATE_EMERGENCY_LAND;
                _navigator->set_commander_request_updated();
                break;
            }
		}
	}
}
