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
 * @file abs_follow.h
 *
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#ifndef NAVIGATOR_LEASHED_FOLLOW_H
#define NAVIGATOR_LEASHED_FOLLOW_H

#include <controllib/blocks.hpp>
#include <controllib/block/BlockParam.hpp>

#include <uORB/uORB.h>

#include "navigator_mode.h"
#include "mission_block.h"

class Leashed : public MissionBlock
{
public:
	Leashed(Navigator *navigator, const char *name);

	~Leashed();

	virtual void on_inactive();

	virtual void on_activation();

	virtual void on_active();

	virtual void execute_vehicle_command();

private:

	math::Vector<3>  _afollow_offset;			/**< offset from target for AFOLLOW mode */
	double	_target_lat;		/**< prediction for target latitude */
	double	_target_lon;		/**< prediction for target longitude */
	float	_target_alt;		/**< prediction for target altitude */
	double	_vehicle_lat;		/**< prediction for vehicle latitude */
	double	_vehicle_lon;		/**< prediction for vehicle longitude */
	float	_vehicle_alt;		/**< prediction for vehicle altitude */
	float 	_init_alt; 			/**< initial altitude on leased follow start > */
    bool    _ready_to_follow;   /**< true if ready, false if not > */
    double _first_leash_point[3]; /** {lat, lon, alt} of first point setted from target */
    double _last_leash_point[3]; /** {lat, lon, alt} of last point setted from target */
	math::Vector<3>	_target_vel;	/**< target velocity vector */
	hrt_abstime _t_prev;

};

#endif
