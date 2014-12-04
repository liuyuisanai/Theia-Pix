/***************************************************************************
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
 * @file loiter.h
 *
 * Helper class to loiter
 *
 * @author Julian Oes <julian@oes.ch>
 */

#ifndef NAVIGATOR_LOITER_H
#define NAVIGATOR_LOITER_H

#include <mathlib/mathlib.h>
#include <geo/geo.h>
#include <controllib/blocks.hpp>
#include <controllib/block/BlockParam.hpp>
#include <uORB/topics/vehicle_command.h>

#include "navigator_mode.h"
#include "mission_block.h"

class Loiter : public MissionBlock
{
public:
	Loiter(Navigator *navigator, const char *name);

	~Loiter();

	virtual void on_inactive();

	virtual void on_activation();

	virtual void on_active();

	virtual void execute_vehicle_command();

private:

	enum LOITER_SUB_MODE {
		LOITER_SUB_MODE_LANDED = 0, 		// vehicle on ground
		LOITER_SUB_MODE_AIM_AND_SHOOT,		// aim and shoot
		LOITER_SUB_MODE_LOOK_DOWN, 			// look down
		LOITER_SUB_MODE_GO_TO_POSITION, 	// go to position
		LOITER_SUB_MODE_LANDING, 			// vehicle is landing
		LOITER_SUB_MODE_TAKING_OFF,			// vehicle is taking off

	} loiter_sub_mode;

	void execute_command_in_landed(vehicle_command_s cmd);
	void execute_command_in_aim_and_shoot(vehicle_command_s cmd);
	void execute_command_in_look_down(vehicle_command_s cmd);
	void execute_command_in_go_to_position(vehicle_command_s cmd);
	void execute_command_in_landing(vehicle_command_s cmd);
	void execute_command_in_taking_off(vehicle_command_s cmd);

	void set_sub_mode(LOITER_SUB_MODE new_sub_mode, uint8_t reset_setpoint);

	bool flag_sub_mode_goal_reached;
};

#endif
