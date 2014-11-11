/****************************************************************************
 *
 *   Copyright (C) 2012 - 2014 PX4 Development Team. All rights reserved.
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
 * @file commander_request_inav.h
 * Definition of the commander_request_inav uORB topic.
 *
 * Commander_requests_inav are requests to commander for vehicle status changes.
 *
 * Commander is the responsible app for managing system related tasks
 * and setting vehicle statuses other apps must make request for commander
 * if system related task or system state change is needed.
 *
 * Commander will process this request and decide what to do with it.
 *
 * @author Maxims Shvetsov <maxim.shvetsov@airdog.com>
 */

#ifndef COMMANDER_REQUEST_INAV_H_
#define COMMANDER_REQUEST_INAV_H_

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"
#include "vehicle_status.h"

/**
 * @addtogroup topics @{
 */

/**
 * Request type.
 */
typedef enum {
	V_DISARM_INAV = 0,                   // Request to disarm vehicle
    AIRD_STATE_CHANGE_INAV           // Request to change airdog_state
} request_type_inav_t;

struct commander_request_inav_s {
	request_type_inav_t request_type;

	main_state_t main_state;
    airdog_state_t airdog_state;

};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(commander_request_inav);

#endif
