#pragma once

#include <drivers/drv_hrt.h>

#include <commander/px4_custom_mode.h>
#include <systemlib/param/param.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_command.h>

__EXPORT void send_command(enum REMOTE_CMD command);
__EXPORT void send_arm_command();

class DroneStatus;

class DroneCommand
{
public:
	DroneCommand();
    static DroneCommand* instance();
	void send_command(REMOTE_CMD);
	void send_come_to_me_command();
	void send_arm_command(const DroneStatus &);
	void send_rtl_command(const DroneStatus &);
private:
    static DroneCommand* _instance;
	param_t param_system_id;
	param_t param_component_id;
	unique_file global_pos_sub;
	void init(struct vehicle_command_s &);
	void send_set_mode(uint8_t, PX4_CUSTOM_MAIN_MODE, int = 0);
};
