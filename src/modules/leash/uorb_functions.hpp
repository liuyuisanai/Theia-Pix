#pragma once

#include <drivers/drv_hrt.h>

#include <commander/px4_custom_mode.h>
#include <systemlib/param/param.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/leash_status.h>

#include "kbd_defines.hpp"
#include "unique_file.hpp"

namespace kbd_handler
{

__EXPORT void send_command(enum REMOTE_CMD command);
__EXPORT void send_arm_command();

class DroneStatus;

class DroneCommand
{
public:
	DroneCommand();
	void send_command(REMOTE_CMD);
	void send_come_to_me_command();
	void send_arm_command(const DroneStatus &);
	void send_rtl_command(const DroneStatus &);
private:
	param_t param_system_id;
	param_t param_component_id;
	unique_file global_pos_sub;
	void init(struct vehicle_command_s &);
	void send_set_mode(uint8_t, PX4_CUSTOM_MAIN_MODE, int = 0);
};

class DroneStatus
{
public:
	DroneStatus();
	~DroneStatus();

	inline bool copter_state_has_changed() const { return status_changed; }
	bool active() const;
	bool in_air() const;
	bool armed() const;
	bool ready_to_arm() const;

	void update(hrt_abstime);
private:
	friend class DroneCommand;
	int sub;
	struct airdog_status_s airdog_status;
	unsigned heartbeat_age_us;
	bool signal_timeout;
	bool status_changed;
	void read_status(struct airdog_status_s &) const;
};

class LeashStatus
{
public:
        LeashStatus();
        void set_mode(kbd_handler::ModeId mode);
private:
	orb_advert_t pub;
};

} // end of namespace kbd_handler
