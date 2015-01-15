#pragma once

#include <commander/px4_custom_mode.h>
#include <systemlib/param/param.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_command.h>

namespace airleash {

__EXPORT void send_command(enum REMOTE_CMD command);
__EXPORT void send_arm_command();

class DroneStatus;

class DroneCommand
{
public:
	DroneCommand();
	void send_command(REMOTE_CMD);
	void send_arm_command(const DroneStatus &);
	void send_rtl_command(const DroneStatus &);
private:
	param_t param_system_id;
	param_t param_component_id;
	void init(struct vehicle_command_s &);
	void send_set_mode(uint8_t, PX4_CUSTOM_MAIN_MODE, int = 0);
};

class DroneStatus
{
public:
	DroneStatus();
	~DroneStatus();

	bool copter_state_has_changed() const;
	bool active() const;
	bool in_air() const;
	bool armed() const;
	bool ready_to_arm() const;

	void update();
private:
	friend class DroneCommand;
	int sub;
	struct airdog_status_s airdog_status;
	bool status_changed;
	void read_status(struct airdog_status_s &) const;
};

} // end of namespace airleash
