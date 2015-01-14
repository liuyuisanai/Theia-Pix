#include <commander/px4_custom_mode.h>
#include <uORB/uORB.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/vehicle_status.h>

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

static void
orb_read_once(const orb_metadata * const id, void * const x)
{
	auto sub = orb_subscribe(id);
	orb_copy(id, sub, x);
	orb_unsubscribe(sub);
}

static void
set_peers(struct vehicle_command_s & cmd)
{
	struct vehicle_status_s vehicle_status;
	orb_read_once(ORB_ID(vehicle_status), &vehicle_status);

	cmd.source_system = vehicle_status.system_id;
	cmd.source_component = vehicle_status.component_id;

// TODO add parameters AD_VEH_SYSID, AD_VEH_COMP to set target id

	cmd.target_system = 1;
	cmd.target_component = 50;
}

static void
send_command(enum REMOTE_CMD command)
{
	struct vehicle_command_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	cmd.command = VEHICLE_CMD_NAV_REMOTE_CMD;
	cmd.param1 = command;

	set_peers(cmd);
	orb_advertise(ORB_ID(vehicle_command), &cmd);
}

static void
send_set_mode(uint8_t base_mode, enum PX4_CUSTOM_MAIN_MODE custom_main_mode)
{
	struct vehicle_command_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	cmd.command = VEHICLE_CMD_DO_SET_MODE;
	cmd.param1 = base_mode;
	cmd.param2 = custom_main_mode;

	set_peers(cmd);
	orb_advertise(ORB_ID(vehicle_command), &cmd);
}

static bool
is_drone_armed()
{
	struct airdog_status_s airdog_status;
	orb_read_once(ORB_ID(airdog_status), &airdog_status);
	return airdog_status.base_mode & MAV_MODE_FLAG_SAFETY_ARMED;
}

static bool
is_drone_active()
{
	struct airdog_status_s airdog_status;
	orb_read_once(ORB_ID(airdog_status), &airdog_status);
	return airdog_status.timestamp > 0;
}

static void
send_arm_command()
{
	struct airdog_status_s airdog_status;
	orb_read_once(ORB_ID(airdog_status), &airdog_status);

	uint8_t mode = airdog_status.base_mode
			| MAV_MODE_FLAG_SAFETY_ARMED
			| MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
	send_set_mode(mode, PX4_CUSTOM_MAIN_MODE_AUTO);
}

} // end of namespace airleash
