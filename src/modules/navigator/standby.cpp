/**
 * @file standby.cpp
 *
 * Navigation mode to support grounded state (armed and waiting for take-off)
 *
 * @author Ilya Nevdah <ilya@airdog.com>
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

#include "standby.h"
#include "navigator.h"

Standby::Standby(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name)
{
    updateParameters();
}

Standby::~Standby()
{
}

void
Standby::on_inactive() {
}

void
Standby::on_activation() {    
    mavlink_log_info(_navigator->get_mavlink_fd(), "Activating Standby navigation mode");
}

void
Standby::on_active() {
	if (update_vehicle_command())
			execute_vehicle_command();
}

void
Standby::execute_vehicle_command() {

	vehicle_command_s cmd = _vcommand;

	if (cmd.command == VEHICLE_CMD_NAV_REMOTE_CMD) {

		int remote_cmd = cmd.param1;
        switch(remote_cmd) {
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
            }
        }
    }
}
