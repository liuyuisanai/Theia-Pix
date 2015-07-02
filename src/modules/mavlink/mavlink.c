/****************************************************************************
 *
 *   Copyright (c) 2012-2014 PX4 Development Team. All rights reserved.
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
 * @file mavlink.c
 * Adapter functions expected by the protocol library
 *
 * @author Lorenz Meier <lm@inf.ethz.ch>
 */

#include <nuttx/config.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "mavlink_bridge_header.h"
#include <systemlib/param/param.h>

/* define MAVLink specific parameters */
/**
 * MAVLink system ID
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_SYS_ID, 1);
/**
 * MAVLink component ID
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_COMP_ID, 50);
/**
 * MAVLink type
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_TYPE, MAV_TYPE_FIXED_WING);
/**
 * Use/Accept HIL GPS message (even if not in HIL mode)
 * If set to 1 incomming HIL GPS messages are parsed.
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_USEHILGPS, 0);
/**
 * Forward external setpoint messages
 * If set to 1 incomming external setpoint messages will be directly forwarded to the controllers if in offboard
 * control mode
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_FWDEXTSP, 1);

/**
 * Indicates whether to use minimalistic stream configuration
 * If set to 1 rcS will start mavlink with very few streams allowing
 * more bandwidth for target messages
 * If set to 2, then default streams (like heartbeat and command)
 * will be disabled too allowing usage of combo-message
 * If set to 3, then default streams will be disabled and rcS will
 * start streams with parameter frequency
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_MINIMALISTIC, 0);

/**
 * Indicates whether mavlink_log_info (warning and critical) functions are enabled
 * On 0 - disabled, otherwise - enabled
 * @group MAVLink
 */
PARAM_DEFINE_INT32(MAV_ENABLE_LOG, 1);

/**
 * Stream frequency parameters used in MAV_MINIMALISTIC = 3
 * If set to -1 default frequency is used
 * If set to 0 the stream will be off
 * If set to positive value it will be used as frequency
 * @group MAVLink
 */
PARAM_DEFINE_FLOAT(MAV_FQ_HRT, -1.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_PARAMS, -1.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_COMMAND, -1.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_STATUS, 0.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_GPS_RAW, 0.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_GPOS, -1.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_COMBO, 0.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_TEXT, -1.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_ATTITUDE, 0.0f);
PARAM_DEFINE_FLOAT(MAV_FQ_TRAJ, -1.0f);

mavlink_system_t mavlink_system = {
	100,
	50,
	MAV_TYPE_FIXED_WING,
	0,
	0,
	0
}; // System ID, 1-255, Component/Subsystem ID, 1-255

/*
 * Internal function to give access to the channel status for each channel
 */
extern mavlink_status_t *mavlink_get_channel_status(uint8_t channel)
{
	static mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];
	return &m_mavlink_status[channel];
}

/*
 * Internal function to give access to the channel buffer for each channel
 */
extern mavlink_message_t *mavlink_get_channel_buffer(uint8_t channel)
{
	static mavlink_message_t m_mavlink_buffer[MAVLINK_COMM_NUM_BUFFERS];
	return &m_mavlink_buffer[channel];
}
