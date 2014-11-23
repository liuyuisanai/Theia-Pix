#ifndef AIRDOG_H
#define AIRDOG_H

#include <nuttx/config.h>
#include <nuttx/sched.h>
#include <nuttx/wqueue.h>
#include <nuttx/clock.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <systemlib/systemlib.h>
#include <systemlib/param/param.h>
#include <systemlib/err.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/vehicle_control_mode.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/debug_key_value.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/airdog_path_log.h>

#include <drivers/device/i2c.h>
#include <drivers/drv_rgbled.h>
#include <drivers/drv_tone_alarm.h>
#include <drivers/drv_hrt.h>
#include <commander/px4_custom_mode.h>
#include <commander/commander_helper.h>
#include <board_config.h>

#include <mavlink/mavlink_log.h>

#include "paramhandler.h"
#include "button_controller.h"
#include "i2c_controller.h"
#include "i2c_display_controller.h"
#include "menu_controller.h"
#include "common.h"

#define LISTENER_ADDR 0x20 /**< I2C adress of our button i2c controller */
#define DISPLAY_ADDR 0x38 /**< I2C adress of our display i2c controller */

class cAirdog
{
public:
	cAirdog();
	~cAirdog();

	void start();
	void cycle();

	struct work_s work;

	bool button_pressed_i2c(uint8_t button, hrt_abstime time);
	bool button_clicked_i2c(uint8_t button, bool long_press);

private:
	enum MAV_MODE_FLAG {
		MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = 1,		/* 0b00000001 Reserved for future use. | */
		MAV_MODE_FLAG_TEST_ENABLED = 2,				/* 0b00000010 system has a test mode enabled. This flag is intended for temporary system tests and should not be used for stable implementations. | */
		MAV_MODE_FLAG_AUTO_ENABLED = 4,				/* 0b00000100 autonomous mode enabled, system finds its own goal positions. Guided flag can be set or not, depends on the actual implementation. | */
		MAV_MODE_FLAG_GUIDED_ENABLED = 8,			/* 0b00001000 guided mode enabled, system flies MISSIONs / mission items. | */
		MAV_MODE_FLAG_STABILIZE_ENABLED = 16,		/* 0b00010000 system stabilizes electronically its attitude (and optionally position). It needs however further control inputs to move around. | */
		MAV_MODE_FLAG_HIL_ENABLED = 32,				/* 0b00100000 hardware in the loop simulation. All motors / actuators are blocked, but internal software is full operational. | */
		MAV_MODE_FLAG_MANUAL_INPUT_ENABLED = 64,	/* 0b01000000 remote control input is enabled. | */
		MAV_MODE_FLAG_SAFETY_ARMED = 128,			/* 0b10000000 MAV safety set to armed. Motors are enabled / running / can start. Ready to fly. | */
	};

    typedef enum MAV_STATE
    {
        MAV_STATE_UNINIT=0, /* Uninitialized system, state is unknown. | */
        MAV_STATE_BOOT=1, /* System is booting up. | */
        MAV_STATE_CALIBRATING=2, /* System is calibrating and not flight-ready. | */
        MAV_STATE_STANDBY=3, /* System is grounded and on standby. It can be launched any time. | */
        MAV_STATE_ACTIVE=4, /* System is active and might be already airborne. Motors are engaged. | */
        MAV_STATE_CRITICAL=5, /* System is in a non-normal flight mode. It can however still navigate. | */
        MAV_STATE_EMERGENCY=6, /* System is in a non-normal flight mode. It lost control over parts or over the whole airframe. It is in mayday and going down. | */
        MAV_STATE_POWEROFF=7, /* System just initialized its power-down sequence, will shut down now. | */
        MAV_STATE_ENUM_END=8, /*  | */
    };

    enum BUTTON_STATE {
        BUTTON_STATE_DEFAULT = 0,
        BUTTON_STATE_CHOOSE_FUNCTION,
        BUTTON_STATE_CONFIRM_TAKEOFF
    } current_button_state;


	void send_set_mode(uint8_t base_mode, enum PX4_CUSTOM_MAIN_MODE custom_main_mode, int mode_args = 0);
	void send_set_auto_mode(uint8_t base_mode, enum PX4_CUSTOM_SUB_MODE_AUTO custom_sub_mode_auto);
	void send_command(enum REMOTE_CMD command);
	// void send_set_state(enum NAV_STATE state, enum AUTO_MOVE_DIRECTION direction);
	// void send_set_move(enum AUTO_MOVE_DIRECTION direction);
	void send_save_params();
	void send_set_parameter_cmd(char *param_name, int int_val, bool should_save);
	void send_get_parameter_value_cmd(char *param_name);
	void send_record_path_cmd(bool start);
	void set_land_mode();
	void display_drone_state();
	void load_param(int menu_param);
	void set_param_if_updated();
	void handle_takeoff();
	void display_discharged_mah();
    void set_current_button_state(BUTTON_STATE button_state);

	bool running;

	uint8_t base_mode;
	int airdog_status_sub;
	int vehicle_status_sub;

	orb_advert_t cmd_pub;
	orb_advert_t cmd_log_start;

	int buzzer;

	I2C_CONTROLLER *pi2c_ctrl;
	I2C_DISPLAY_CONTROLLER *pi2c_disp_ctrl;
	MENU_CONTROLLER *pmenu_ctrl;
	cParamHandler *pparam_handler;
	cButtonController *pbutton_ctrl;

	int mavlink_fd;
	bool hil;
	bool armed;
	bool drone_active;
	bool log_running;

	struct airdog_status_s airdog_status;
	struct vehicle_status_s vehicle_status;

	uint64_t last_drone_timestamp;
	uint64_t takeoff_request_time;

	bool takeoff_requested;

	param_t param_bat_warn;
	param_t param_bat_failsafe;
	param_t param_trainer_id;

	float bat_warning_level;
	float bat_critical_level;
	bool rtl_triggered_from_battery;

	int32_t trainer_remote_id;

	bool pitch_down;

	int battery_warning_count;
};

#endif // AIRDOG_H
