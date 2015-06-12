#include <cstdio>
#include <cstring>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_status.h>

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

namespace airleash {

DroneCommand::
DroneCommand()
	: param_system_id(param_find("MAV_SYS_ID"))
	, param_component_id(param_find("MAV_COMP_ID"))
{}

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
send_come_to_me_command(double lat, double lon)
{
	struct vehicle_command_s cmd;
	init(cmd);

	cmd.command = VEHICLE_CMD_NAV_REMOTE_CMD;
	cmd.param1 = REMOTE_CMD_COME_TO_ME;
	cmd.param5 = lat;
	cmd.param6 = lon;

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

LeashStatus::LeashStatus() :
        leash_status_pub(-1)
{
	struct leash_status_s l_status;
	l_status.menu_mode = (int8_t) kbd_handler::ModeId::NONE;
	leash_status_pub = orb_advertise(ORB_ID(leash_status), &l_status);
}

void
LeashStatus::set_mode(kbd_handler::ModeId mode) {
    struct leash_status_s l_status;
    l_status.menu_mode = (int8_t) mode; //TODO[max] key_mode
    orb_publish(ORB_ID(leash_status), leash_status_pub, &l_status);
}

DroneStatus::DroneStatus()
	: sub(orb_subscribe(ORB_ID(airdog_status)))
{}

DroneStatus::~DroneStatus()
{
	orb_unsubscribe(sub);
}

void
DroneStatus::update(hrt_abstime now)
{
	struct airdog_status_s x = airdog_status;
	orb_copy(ORB_ID(airdog_status), sub, &x);

	heartbeat_age_us = now - x.timestamp;
	bool x_timeout = HEARTBEAT_TIMEOUT_us < heartbeat_age_us;

	status_changed = (
		   (signal_timeout  != x_timeout)
		or (x.main_mode     != airdog_status.main_mode)
		or (x.sub_mode      != airdog_status.sub_mode)
		or (x.state_main    != airdog_status.state_main)
		or (x.state_aird    != airdog_status.state_aird)
		or (x.system_status != airdog_status.system_status)
		or (x.base_mode     != airdog_status.base_mode)
	);

	if (status_changed)
	{
		fprintf(stderr, "DroneStatus\n");
		fprintf(stderr, "Heartbeat age %uus\n", heartbeat_age_us);
		fprintf(stderr, "States %x %x %x %x Base mode %x System status %x Timeout %i\n",
			airdog_status.main_mode, airdog_status.sub_mode,
			airdog_status.state_main, airdog_status.state_aird,
			airdog_status.base_mode, airdog_status.system_status,
			signal_timeout);
		fprintf(stderr, "%i ==>  %x %x %x %x Base mode %x System status %x Timeout %i\n",
			status_changed,
			x.main_mode, x.sub_mode, x.state_main, x.state_aird, x.base_mode, x.system_status,
			x_timeout);
	}

	airdog_status = x;
	signal_timeout = x_timeout;
}

bool
DroneStatus::active() const
{
	bool r = not signal_timeout;
	fprintf(stderr, "DroneStatus %s.\n", r ? "is active." : "is NOT active.");
	return r;
}

bool
DroneStatus::armed() const
{
	bool r = airdog_status.base_mode & MAV_MODE_FLAG_SAFETY_ARMED;
	fprintf(stderr, "DroneStatus %s.\n", r ? "is armed." : "is NOT armed.");
	return r;
}

bool
DroneStatus::in_air() const
{
	bool r = airdog_status.state_aird > AIRD_STATE_LANDED;
	fprintf(stderr, "DroneStatus %s.\n", r ? "is in air" : "is NOT in air");
	return r;
}

bool
DroneStatus::ready_to_arm() const
{
	bool r = not in_air();
	fprintf(stderr, "DroneStatus %s.\n", r ? "is ready to arm" : "is NOT ready to arm");
	return r;
}

} // end of namespace airleash
