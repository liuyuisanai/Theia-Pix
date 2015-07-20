#include <cstdio>
#include <cstring>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/vehicle_global_position.h>

#include "debug.hpp"
#include "settings.hpp"
#include "uorb_functions.hpp"

enum MAV_MODE_FLAG {
	/*
	 * enum MAV_MODE_FLAG lives in mavlink/custom/headers/common/common.h,
	 * but it bloodily could not be included.
	 */
	MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = 1,
	MAV_MODE_FLAG_TEST_ENABLED = 2,
	MAV_MODE_FLAG_AUTO_ENABLED = 4,
	MAV_MODE_FLAG_GUIDED_ENABLED = 8,
	MAV_MODE_FLAG_STABILIZE_ENABLED = 16,
	MAV_MODE_FLAG_HIL_ENABLED = 32,
	MAV_MODE_FLAG_MANUAL_INPUT_ENABLED = 64,
	MAV_MODE_FLAG_SAFETY_ARMED = 128,
};

/*
 * DroneCommand
 */
DroneCommand* DroneCommand::instance()
{
    if (_instance == nullptr) 
    {
        _instance = new DroneCommand();
    }
    return _instance;
}

DroneCommand::
DroneCommand()
	: param_system_id(param_find("MAV_SYS_ID"))
	, param_component_id(param_find("MAV_COMP_ID"))
{
	global_pos_sub = orb_subscribe(ORB_ID(vehicle_global_position));
}

void DroneCommand::
init(struct vehicle_command_s & cmd)
{
	memset(&cmd, 0, sizeof(cmd));

	cmd.source_system = param_system_id;
	cmd.source_component = param_component_id;

// TODO add parameters AD_VEH_SYSID, AD_VEH_COMP to set target id

	cmd.target_system = 1;
	cmd.target_component = 50;
}

void DroneCommand::
send_command(enum REMOTE_CMD command)
{
	struct vehicle_command_s cmd;
	init(cmd);

	cmd.command = VEHICLE_CMD_NAV_REMOTE_CMD;
	cmd.param1 = command;

	orb_advertise(ORB_ID(vehicle_command), &cmd);
	say_f("Sent remote cmd %i", command);
}

void DroneCommand::
send_come_to_me_command()
{
    vehicle_global_position_s global_pos;
    orb_copy(ORB_ID(vehicle_global_position),
    global_pos_sub.get(), &global_pos);

	struct vehicle_command_s cmd;
	init(cmd);

	cmd.command = VEHICLE_CMD_NAV_REMOTE_CMD;
	cmd.param1 = REMOTE_CMD_COME_TO_ME;
	cmd.param5 = global_pos.lat;
	cmd.param6 = global_pos.lon;

	orb_advertise(ORB_ID(vehicle_command), &cmd);
	say("Sent remote cmd Come to me");
}


void DroneCommand::
send_set_mode(uint8_t base_mode, enum PX4_CUSTOM_MAIN_MODE custom_main_mode, int param3)
{
	struct vehicle_command_s cmd;
	init(cmd);

	cmd.command = VEHICLE_CMD_DO_SET_MODE;
	cmd.param1 = base_mode;
	cmd.param2 = custom_main_mode;
	cmd.param3 = param3;

	orb_advertise(ORB_ID(vehicle_command), &cmd);
}

void DroneCommand::
send_arm_command(const DroneStatus & s)
{
	uint8_t mode = s.airdog_status.base_mode
			| MAV_MODE_FLAG_SAFETY_ARMED
			| MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
	send_set_mode(mode, PX4_CUSTOM_MAIN_MODE_LOITER, 1);

	//say("ARM command sent.");
}

void DroneCommand::
send_rtl_command(const DroneStatus & s)
{
	uint8_t mode = s.airdog_status.base_mode
			| MAV_MODE_FLAG_SAFETY_ARMED
			| MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
	send_set_mode(mode, PX4_CUSTOM_MAIN_MODE_RTL);

	//say("RTL command sent.");
}
