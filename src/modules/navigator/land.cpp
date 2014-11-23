/**
 * @file land.cpp
 *
 * Navigation mode to simply land the vehicle.
 *
 * @author Martins Frolovs <martins.f@airdog.vom>
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

#include "land.h"
#include "navigator.h"

Land::Land(Navigator *navigator, const char *name) :
	MissionBlock(navigator, name)
{
    updateParameters();
}

Land::~Land()
{
}

void
Land::on_inactive(){
}

void
Land::on_activation(){
    

    mavlink_log_info(_navigator->get_mavlink_fd(), "Activating land !");

    landing_finished = false;
    land();

}

void
Land::on_active(){

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
    if (!landing_finished && check_current_pos_sp_reached()){
        landing_finished = true;

        mavlink_log_info(_navigator->get_mavlink_fd(), "Setpoint reached !");
        warnx("---- Landed, disarming... ----");
        disarm();
    }

	if (!landing_finished && update_vehicle_command() )
			execute_vehicle_command();

}

void
Land::execute_vehicle_command() {

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
