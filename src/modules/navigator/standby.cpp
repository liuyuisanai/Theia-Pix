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
}
