#include "uorb_functions.h"

#include <commander/px4_custom_mode.h>

#include <stdio.h>

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

void sendAirDogCommnad(enum VEHICLE_CMD command,
                      float param1,
                      float param2,
                      float param3,
                      float param4,
                      double param5,
                      double param6,
                      float param7
)
{
    struct vehicle_command_s vehicle_command;
    static orb_advert_t to_vehicle_command = 0;


    printf("sendAirDogCommnad cmd %d: %.3f %.3f %.3f %.3f %.3f\n", (int)command,
           (double)param1, (double)param2, (double)param3,
           (double)param4, (double)param5);

    vehicle_command.command = command;
    vehicle_command.param1 = param1;
    vehicle_command.param2 = param2;
    vehicle_command.param3 = param3;
    vehicle_command.param4 = param4;
    vehicle_command.param5 = param5;
    vehicle_command.param6 = param6;
    vehicle_command.param7 = param7;

    vehicle_command.target_system = 1;
    vehicle_command.target_component = 50;

    if (to_vehicle_command > 0)
    {
        orb_publish(ORB_ID(vehicle_command), to_vehicle_command, &vehicle_command);
    }
    else
    {
        to_vehicle_command = orb_advertise(ORB_ID(vehicle_command), &vehicle_command);
    }
}

void send_arm_command(const airdog_status_s &s)
{
    struct vehicle_command_s cmd;

	uint8_t mode = s.base_mode
			| MAV_MODE_FLAG_SAFETY_ARMED
			| MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

    cmd.command = VEHICLE_CMD_DO_SET_MODE;
    cmd.param1 = mode;
    cmd.param2 = PX4_CUSTOM_MAIN_MODE_LOITER;
    cmd.param3 = 1;
    cmd.target_system = 1;
    cmd.target_component = 50;

    DOG_PRINT("[leash_app]{UORB} sending arm command\n");
    orb_advertise(ORB_ID(vehicle_command), &cmd);
}

void send_rtl_command(const airdog_status_s &s)
{
    struct vehicle_command_s cmd;

	uint8_t mode = s.base_mode
			| MAV_MODE_FLAG_SAFETY_ARMED
			| MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

    cmd.command = VEHICLE_CMD_DO_SET_MODE;
    cmd.param1 = mode;
    cmd.param2 = PX4_CUSTOM_MAIN_MODE_RTL;
    cmd.param3 = 0;
    cmd.target_system = 1;
    cmd.target_component = 50;

    DOG_PRINT("[leash_app]{UORB} sending rtl command\n");
    orb_advertise(ORB_ID(vehicle_command), &cmd);
}
