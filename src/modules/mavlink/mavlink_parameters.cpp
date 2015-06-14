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
 * @file mavlink_parameters.cpp
 * Mavlink parameters manager implementation.
 *
 * @author Anton Babushkin <anton.babushkin@me.com>
 */

#include <stdio.h>
#include <uORB/topics/get_drone_parameter.h>
#include <uORB/topics/set_drone_parameter.h>

#include "mavlink_parameters.h"
#include "mavlink_main.h"

MavlinkParametersManager::MavlinkParametersManager(Mavlink *mavlink) : MavlinkStream(mavlink),
	_send_all_index(-1)
{


}

unsigned
MavlinkParametersManager::get_size()
{
	if (_send_all_index >= 0) {
		return MAVLINK_MSG_ID_PARAM_VALUE_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES;

	} else {
		return 0;
	}
}

void
MavlinkParametersManager::handle_message(const mavlink_message_t *msg)
{
	switch (msg->msgid) {
	case MAVLINK_MSG_ID_PARAM_REQUEST_LIST: {


			/* request all parameters */
			mavlink_param_request_list_t req_list;
			mavlink_msg_param_request_list_decode(msg, &req_list);

			if (req_list.target_system == mavlink_system.sysid &&
			    (req_list.target_component == mavlink_system.compid || req_list.target_component == MAV_COMP_ID_ALL)) {

				_send_all_index = 0;
				_mavlink->send_statustext_info("[pm] sending list");
			}
			break;
		}

	case MAVLINK_MSG_ID_PARAM_SET: {
			/* set parameter */
			if (msg->msgid == MAVLINK_MSG_ID_PARAM_SET) {
				mavlink_param_set_t set;
				mavlink_msg_param_set_decode(msg, &set);

				if (set.target_system == mavlink_system.sysid &&
				    (set.target_component == mavlink_system.compid || set.target_component == MAV_COMP_ID_ALL)) {

					/* local name buffer to enforce null-terminated string */
					char name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1];
					strncpy(name, set.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);
					/* enforce null termination */
					name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN] = '\0';
					/* attempt to find parameter, set and send it */
					param_t param = param_find(name);

					if (param == PARAM_INVALID) {
						char buf[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN];
						sprintf(buf, "[pm] unknown param: %s", name);
						_mavlink->send_statustext_info(buf);

					} else {
						/* set and send parameter */
						param_set(param, &(set.param_value));
						send_param(param);
					}
				}
			}
			break;
		}

	case MAVLINK_MSG_ID_PARAM_REQUEST_READ: {
			/* request one parameter */

			mavlink_param_request_read_t req_read;
			mavlink_msg_param_request_read_decode(msg, &req_read);

			if (req_read.target_system == mavlink_system.sysid &&
			    (req_read.target_component == mavlink_system.compid || req_read.target_component == MAV_COMP_ID_ALL)) {

				/* when no index is given, loop through string ids and compare them */
				if (req_read.param_index < 0) {
					/* local name buffer to enforce null-terminated string */
					char name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN + 1];
					strncpy(name, req_read.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);
					/* enforce null termination */
					name[MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN] = '\0';
					/* attempt to find parameter and send it */

					send_param(param_find(name));

				} else {
					/* when index is >= 0, send this parameter again */
					send_param(param_for_index(req_read.param_index));
				}
			}
			break;
		}

      case MAVLINK_MSG_ID_PARAM_VALUE: {

             mavlink_param_value_t param_value_msg;
             mavlink_msg_param_value_decode(msg, &param_value_msg);
             struct pass_drone_param_s cmd;
             memset(&cmd, 0, sizeof(cmd));

             cmd.param_value = param_value_msg.param_value;
             cmd.param_type = param_value_msg.param_type;
             strncpy(cmd.param_id, param_value_msg.param_id, 16);

            if (_mavlink->_pass_drone_parameter_pub < 0)
                _mavlink->_pass_drone_parameter_pub = orb_advertise(ORB_ID(pass_drone_parameter), &cmd);
            else
                orb_publish(ORB_ID(pass_drone_parameter), _mavlink->_pass_drone_parameter_pub, &cmd);

        } break;


	default:
		break;
	}
}

void
MavlinkParametersManager::send(const hrt_abstime t)
{
	/* send all parameters if requested */
	if (_send_all_index >= 0) {
		send_param(param_for_index(_send_all_index));
		_send_all_index++;
		if (_send_all_index >= (int) param_count()) {
			_send_all_index = -1;
		}
	}

    bool updated;
    orb_check(_mavlink->get_drone_parameter_sub, &updated);

    if (updated) {
        get_drone_param_s get_drone_param;

        memset(&get_drone_param, 0, sizeof(get_drone_param));

        orb_copy(ORB_ID(get_drone_parameter), _mavlink->get_drone_parameter_sub , &get_drone_param);

        mavlink_param_request_read_t msg;

        memcpy(msg.param_id, get_drone_param.param_id, sizeof(msg.param_id));
        msg.param_index = get_drone_param.param_index;
        msg.target_component = get_drone_param.target_component;
        msg.target_system = get_drone_param.target_system;
        _mavlink->send_message(MAVLINK_MSG_ID_PARAM_REQUEST_READ, &msg);

    }

    orb_check(_mavlink->set_drone_parameter_sub, &updated);

    if (updated) {
        set_drone_param_s set_drone_param;

        memset(&set_drone_param, 0, sizeof(set_drone_param));

        orb_copy(ORB_ID(set_drone_parameter), _mavlink->set_drone_parameter_sub, &set_drone_param);

        mavlink_param_set_t msg;

        memcpy(msg.param_id, set_drone_param.param_id, sizeof(msg.param_id));
        msg.target_system = set_drone_param.target_system;
        msg.param_value = set_drone_param.param_value;
        msg.param_type = set_drone_param.param_type;

        _mavlink->send_message(MAVLINK_MSG_ID_PARAM_SET, &msg);
    }
}

void
MavlinkParametersManager::send_param(param_t param)
{
	if (param == PARAM_INVALID) {
		return;
	}

	mavlink_param_value_t msg;

	/*
	 * get param value, since MAVLink encodes float and int params in the same
	 * space during transmission, copy param onto float val_buf
	 */
	if (param_get(param, &msg.param_value) != OK) {
		return;
	}

	msg.param_count = param_count();
	msg.param_index = param_get_index(param);

	/* copy parameter name */
	strncpy(msg.param_id, param_name(param), MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);

	/* query parameter type */
	param_type_t type = param_type(param);

	/*
	 * Map onboard parameter type to MAVLink type,
	 * endianess matches (both little endian)
	 */
	if (type == PARAM_TYPE_INT32) {
		msg.param_type = MAVLINK_TYPE_INT32_T;

	} else if (type == PARAM_TYPE_FLOAT) {
		msg.param_type = MAVLINK_TYPE_FLOAT;

	} else {
		msg.param_type = MAVLINK_TYPE_FLOAT;
	}

	_mavlink->send_message(MAVLINK_MSG_ID_PARAM_VALUE, &msg);
}
