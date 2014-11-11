#include "airdog.h"

extern "C" __EXPORT int airdog_main(int argc, char *argv[]);

//For work queue, it requries void function
static void airdog_cycle(void *arg);
static void airdog_start(void *arg);

//For button controller callbacks
static bool airdog_button_pressed_i2c(void *arg, uint8_t button, hrt_abstime time);
static bool airdog_button_clicked_i2c(void *arg, uint8_t button, bool long_press);

cAirdog *g_pAirdog = nullptr;

cAirdog::cAirdog() :
	running(false),

	base_mode(0),

	airdog_status_sub(0),
	vehicle_status_sub(0),

	cmd_pub(-1),
	cmd_log_start(-1),

	buzzer(0),

	pi2c_ctrl(nullptr),
	pi2c_disp_ctrl(nullptr),
	pmenu_ctrl(nullptr),
	pparam_handler(nullptr),
	pbutton_ctrl(nullptr),

	mavlink_fd(0),
	hil(false),
	armed(false),
	drone_active(false),
	log_running(false),

	last_drone_timestamp(0),
	takeoff_request_time(0),

	takeoff_requested(false),

	bat_warning_level(0.0f),
	bat_critical_level(0.0f),
	trainer_remote_id(0),
	pitch_down(false),

	battery_warning_count(0)
{
	memset(&work, 0, sizeof(work));

	memset(&airdog_status, 0, sizeof(airdog_status));
	memset(&vehicle_status, 0, sizeof(vehicle_status));

	memset(&param_bat_warn, 0, sizeof(param_bat_warn));
	memset(&param_bat_failsafe, 0, sizeof(param_bat_failsafe));
	memset(&param_trainer_id, 0, sizeof(param_trainer_id));
}

cAirdog::~cAirdog()
{
	running = false;
	work_cancel(LPWORK, &work);
	if(pmenu_ctrl != nullptr)
		delete pmenu_ctrl;
	if(pi2c_disp_ctrl != nullptr)
		delete pi2c_disp_ctrl;
	if(pi2c_ctrl != nullptr)
		delete pi2c_ctrl;
	if(pbutton_ctrl != nullptr)
		delete pbutton_ctrl;
	if(pparam_handler != nullptr)
		delete pparam_handler;
}

void cAirdog::start()
{
	int ret;

    current_button_state = BUTTON_STATE_DEFAULT;
	base_mode = MAV_MODE_FLAG_SAFETY_ARMED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

	buzzer = open(TONEALARM_DEVICE_PATH, O_WRONLY);

	/* subscribe to vehicle status topic */
	airdog_status_sub = orb_subscribe(ORB_ID(airdog_status));
	vehicle_status_sub = orb_subscribe(ORB_ID(vehicle_status));

	orb_copy(ORB_ID(vehicle_status), vehicle_status_sub, &vehicle_status);

	param_bat_warn = param_find("AIRD_BAT_WARN");
	param_bat_failsafe = param_find("AIRD_BAT_FS");
	param_trainer_id = param_find("AIRD_TRAINER_ID");

	param_get(param_bat_warn, &bat_warning_level);
	param_get(param_bat_failsafe, &bat_critical_level);
	param_get(param_trainer_id, &trainer_remote_id);

	mavlink_fd = open(MAVLINK_LOG_DEVICE, 0);
	mavlink_log_info(mavlink_fd, "[mpc] started");

	//set to true on request (remote button press)
	log_running = false;

	pparam_handler = new cParamHandler();
	if(pparam_handler == nullptr) errx(30, "airdog couldn't create param handler");
	if(!pparam_handler->init()) {
		errx(31, "airdog couldn't init param handler");
	}
	pparam_handler->setupSave(&cmd_pub, vehicle_status.system_id, vehicle_status.component_id);

	pbutton_ctrl = new cButtonController();
	if(pbutton_ctrl == nullptr) errx(38, "airdog couldn't create button controller");
	pbutton_ctrl->setCallback(BS_I2C, BCBT_PRESSED, (void*)airdog_button_pressed_i2c, this);
	pbutton_ctrl->setCallback(BS_I2C, BCBT_CLICKED, (void*)airdog_button_clicked_i2c, this);

	pi2c_ctrl = new I2C_CONTROLLER(PX4_I2C_BUS_EXPANSION, LISTENER_ADDR);
	if(pi2c_ctrl == nullptr) errx(32, "airdog couldn't create i2c controller");
	if((ret = pi2c_ctrl->init(pbutton_ctrl)) != OK) {
		errx(33, "airdog couldn't init i2c controller (%i)", ret);
	}

	pi2c_disp_ctrl = new I2C_DISPLAY_CONTROLLER(PX4_I2C_BUS_EXPANSION, DISPLAY_ADDR);
	if(pi2c_disp_ctrl == nullptr) errx(34, "airdog couldn't create i2c display controller");
	if((ret = pi2c_disp_ctrl->init()) != OK) {
		errx(35, "airdog couldn't init i2c display controller (%i)", ret);
	}

	pi2c_disp_ctrl->clear_display();

	pmenu_ctrl = new MENU_CONTROLLER(pi2c_disp_ctrl, pparam_handler);
	if(pmenu_ctrl == nullptr) errx(36, "airdog couldn't create menu controller");

	/* add worker to queue */
	running = true;
	ret = work_queue(LPWORK, &work, airdog_cycle, this, 0);
	if (ret != OK) {
		running = false;
		errx(4, "airdog couldn't queue cycle work on start");
	}
}

void cAirdog::cycle()
{
	bool updated;
	orb_check(airdog_status_sub, &updated);

	if (updated) {
		pparam_handler->loadCycle();

		orb_copy(ORB_ID(airdog_status), airdog_status_sub, &airdog_status);
		hil = airdog_status.base_mode & MAV_MODE_FLAG_HIL_ENABLED;
		armed = airdog_status.base_mode & MAV_MODE_FLAG_SAFETY_ARMED;
		display_drone_state();

		// handle_takeoff();

		if(!hil) {
			if (airdog_status.battery_remaining < bat_warning_level && battery_warning_count < 3 && airdog_status.battery_remaining!= 0)
			{
				mavlink_log_info(mavlink_fd, "remaining %d", airdog_status.battery_remaining);
				battery_warning_count++;
				ioctl(buzzer, TONE_SET_ALARM, TONE_NOTIFY_NEGATIVE_TUNE);
				pi2c_ctrl->start_blinking_led(I2C_LED_RED, BLINKING_RATE_SLOW);

			} else if (airdog_status.battery_remaining < bat_critical_level && airdog_status.battery_remaining!= 0)
			{
				if (!rtl_triggered_from_battery && airdog_status.sub_mode != PX4_CUSTOM_SUB_MODE_AUTO_RTL && airdog_status.sub_mode != PX4_CUSTOM_SUB_MODE_AUTO_LAND)
				{
					rtl_triggered_from_battery = true;
					// send_set_state(NAV_STATE_RTL, MOVE_NONE);
					pi2c_ctrl->start_blinking_led(I2C_LED_RED, BLINKING_RATE_FAST);
				}
			} else if (airdog_status.battery_remaining > bat_warning_level)
			{
				pi2c_ctrl->stop_blinking_led(I2C_LED_RED, true);
			}
		}

		if (airdog_status.timestamp > 0)
		{
			drone_active = true;
			pi2c_ctrl->set_red_led_on(true);
			last_drone_timestamp = airdog_status.timestamp;
		} else {
			drone_active = false;
			pi2c_ctrl->set_red_led_on(false);
		}
	}

	uint64_t timeDiff = hrt_absolute_time() - last_drone_timestamp;
	//mavlink_log_info(mavlink_fd,"time diff %llu", timeDiff);
	if (timeDiff > 5000000)
	{
		drone_active = false;
		pi2c_ctrl->set_red_led_on(false);
	}

	//warnx("connected %d, armed %d, hil %d, main mode %d, sub_mode %d",drone_active, _armed, _hil, airdog_status.main_mode, airdog_status.sub_mode);

	pi2c_ctrl->cycle();
	pbutton_ctrl->cycle();

	/* repeat cycle at 10 Hz */
	if (running) {
		work_queue(LPWORK, &work, airdog_cycle, this, USEC2TICK(DEVICE_FREQUENCY));
	}
}

void cAirdog::send_set_mode(uint8_t base_mode, enum PX4_CUSTOM_MAIN_MODE custom_main_mode)
{
	struct vehicle_command_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	/* fill command */
	cmd.command = VEHICLE_CMD_DO_SET_MODE;
	cmd.confirmation = false;
	cmd.param1 = base_mode;
	cmd.param2 = custom_main_mode;
	cmd.source_system = vehicle_status.system_id;
	cmd.source_component = vehicle_status.component_id;
	// TODO add parameters AD_VEH_SYSID, AD_VEH_COMP to set target id
	cmd.target_system = 1;
	cmd.target_component = 50;

	if (cmd_pub < 0) {
		cmd_pub = orb_advertise(ORB_ID(vehicle_command), &cmd);
	} else {
		orb_publish(ORB_ID(vehicle_command), cmd_pub, &cmd);
	}
}

void cAirdog::send_set_auto_mode(uint8_t base_mode, enum PX4_CUSTOM_SUB_MODE_AUTO custom_sub_mode_auto)
{
	struct vehicle_command_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	/* fill command */
	cmd.command = VEHICLE_CMD_DO_SET_MODE;
	cmd.confirmation = false;
	cmd.param1 = base_mode;
	cmd.param2 = custom_sub_mode_auto;
	cmd.source_system = vehicle_status.system_id;
	cmd.source_component = vehicle_status.component_id;
	// TODO add parameters AD_VEH_SYSID, AD_VEH_COMP to set target id
	cmd.target_system = 1;
	cmd.target_component = 50;

	if (cmd_pub < 0) {
		cmd_pub = orb_advertise(ORB_ID(vehicle_command), &cmd);
	} else {
		orb_publish(ORB_ID(vehicle_command), cmd_pub, &cmd);
	}
}

void cAirdog::send_command(enum REMOTE_CMD command)
{
	struct vehicle_command_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	/* fill command */
	cmd.param1 = command;
	cmd.command = VEHICLE_CMD_NAV_REMOTE_CMD;
	cmd.confirmation = false;
	cmd.source_system = vehicle_status.system_id;
	cmd.source_component = vehicle_status.component_id;
	cmd.target_system = 1;
	cmd.target_component = 50;

	if (cmd_pub < 0) {
		cmd_pub = orb_advertise(ORB_ID(vehicle_command), &cmd);
	} else {
		orb_publish(ORB_ID(vehicle_command), cmd_pub, &cmd);
	}
}

// void cAirdog::send_set_state(enum NAV_STATE state, enum AUTO_MOVE_DIRECTION direction)
// {
// 	struct vehicle_command_s cmd;
// 	memset(&cmd, 0, sizeof(cmd));

// 	/* fill command */
// 	cmd.command = VEHICLE_CMD_NAV_SET_STATE;
// 	cmd.param1 = state;
// 	cmd.param2 = direction;
// 	cmd.confirmation = false;
// 	cmd.source_system = vehicle_status.system_id;
// 	cmd.source_component = vehicle_status.component_id;
// 	// TODO add parameters AD_VEH_SYSID, AD_VEH_COMP to set target id
// 	cmd.target_system = 1;
// 	cmd.target_component = 50;

// 	if (cmd_pub < 0) {
// 		cmd_pub = orb_advertise(ORB_ID(vehicle_command), &cmd);
// 	} else {
// 		orb_publish(ORB_ID(vehicle_command), cmd_pub, &cmd);
// 	}
// }

// void cAirdog::send_set_move(enum AUTO_MOVE_DIRECTION direction)
// {
// 	if (airdog_status.sub_mode == PX4_CUSTOM_SUB_MODE_AUTO_LOITER) {
// 		send_set_state(NAV_STATE_LOITER, direction);
// 	} else if (airdog_status.sub_mode == PX4_CUSTOM_SUB_MODE_AUTO_AFOLLOW) {
// 		send_set_state(NAV_STATE_AFOLLOW, direction);
// 	} else {
// 		send_set_state(NAV_STATE_LOITER, direction);
// 	}
// }

void cAirdog::send_record_path_cmd(bool start)
{
	struct airdog_path_log_s cmd;
	memset(&cmd, 0, sizeof(cmd));

	/* fill command */
	cmd.start = start;
	cmd.stop = !start;

	if (cmd_log_start < 0) {
		cmd_log_start = orb_advertise(ORB_ID(airdog_path_log), &cmd);
	} else {
		orb_publish(ORB_ID(airdog_path_log), cmd_log_start, &cmd);
	}
}

void cAirdog::set_land_mode()
{
    /*
	float value = pparam_handler->get(PARAM_NAV_LAND_HOME);
	if(value > 0)
		value = 0;
	else
		value = 1;

	if(pparam_handler->send(PARAM_NAV_LAND_HOME, value, true)) {
		if(value > 0)
			pi2c_disp_ctrl->set_symbols(SYMBOL_EMPTY, SYMBOL_H, SYMBOL_E);
		else
			pi2c_disp_ctrl->set_symbols(SYMBOL_EMPTY, SYMBOL_H, SYMBOL_0);
	} else {
		pi2c_disp_ctrl->set_symbols(SYMBOL_E, SYMBOL_R, SYMBOL_R);
	}
    */
}

bool cAirdog::button_pressed_i2c(uint8_t button, hrt_abstime time)
{
	if(pmenu_ctrl->isActive()) {
		pmenu_ctrl->handlePressedButton(button, time);
		return BUTTON_HANDLED;
	}
	return BUTTON_IGNORED;
}


bool cAirdog::button_clicked_i2c(uint8_t button, bool long_press)
{

	if(pmenu_ctrl->isActive()) {
		pmenu_ctrl->handleClickedButton(button/*, long_press*/);
		return BUTTON_HANDLED;
	}

	switch(button) {
		case 0:
			// ON/OFF button
            
            if (current_button_state == BUTTON_STATE_DEFAULT) {
				pmenu_ctrl->open();
            }

			break;
		case 1:
			// DOWN button
            if (current_button_state == BUTTON_STATE_DEFAULT) {
                send_command(REMOTE_CMD_DOWN);
            } else if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){

                uint8_t base_mode = MAV_MODE_FLAG_SAFETY_ARMED | MAV_MODE_FLAG_AUTO_ENABLED;
                if (hil) base_mode |= MAV_MODE_FLAG_HIL_ENABLED;

                send_set_auto_mode(base_mode, PX4_CUSTOM_SUB_MODE_AUTO_RTL);
                set_current_button_state(BUTTON_STATE_DEFAULT);

                set_current_button_state(BUTTON_STATE_DEFAULT);
            }
			break;
		case 2:

			// PLAY button
            //
            //long_press - takeoff/ land
            if (long_press) {
                if (!armed & drone_active){
                    set_current_button_state(BUTTON_STATE_CONFIRM_TAKEOFF);
                } else {
                    send_command(REMOTE_CMD_LAND_DISARM);
                }

            } else {
            
                if (current_button_state == BUTTON_STATE_DEFAULT) {
                    send_command(REMOTE_CMD_PLAY_PAUSE);
                } else if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){
                    set_current_button_state(BUTTON_STATE_DEFAULT);
                } else if (current_button_state == BUTTON_STATE_CONFIRM_TAKEOFF) {
                    set_current_button_state(BUTTON_STATE_DEFAULT);
                }
            
            }

			break;
		case 3:
			// UP button

            if (current_button_state == BUTTON_STATE_DEFAULT) {
                send_command(REMOTE_CMD_UP);
            }

			break;
		case 4:
			// CENTER button
            if (current_button_state == BUTTON_STATE_CONFIRM_TAKEOFF){

                set_current_button_state(BUTTON_STATE_DEFAULT);

                uint8_t base_mode = MAV_MODE_FLAG_SAFETY_ARMED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
                if (hil) base_mode |= MAV_MODE_FLAG_HIL_ENABLED;
				send_set_mode(base_mode, PX4_CUSTOM_MAIN_MODE_LOITER);
                usleep(100000);
                send_command(REMOTE_CMD_TAKEOFF);
            
            } else if (current_button_state == BUTTON_STATE_DEFAULT) {
                set_current_button_state(BUTTON_STATE_CHOOSE_FUNCTION);

            } else if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){

                send_command(REMOTE_CMD_COME_TO_ME);
                set_current_button_state(BUTTON_STATE_DEFAULT);
            }

			break;
		case 5:
			// CENTER DOWN
            if (current_button_state == BUTTON_STATE_DEFAULT) {

			    send_command(REMOTE_CMD_CLOSER);

            } else if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){

                send_command(REMOTE_CMD_LOOK_DOWN);
                set_current_button_state(BUTTON_STATE_DEFAULT);
            }

			break;
		case 6:
			// CENTER RIGHT
            if (current_button_state == BUTTON_STATE_DEFAULT) {
                send_command(REMOTE_CMD_RIGHT);
            }
			break;
		case 7:
			// CENTER UP
            if (current_button_state == BUTTON_STATE_DEFAULT) {
                send_command(REMOTE_CMD_FURTHER);
            }		
            break;
		case 8:
			// CENTER LEFT
            if (current_button_state == BUTTON_STATE_DEFAULT) {
                send_command(REMOTE_CMD_LEFT);
            } else if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){
            
                if (!log_running) {
                    send_record_path_cmd(true);
                    log_running = true;
                    pi2c_disp_ctrl->set_symbols(SYMBOL_L, SYMBOL_0, SYMBOL_EMPTY);
                 } else {
                    send_record_path_cmd(false);
                    log_running = false;
                    pi2c_disp_ctrl->set_symbols(SYMBOL_MINUS, SYMBOL_L, SYMBOL_0);
                 }
                set_current_button_state(BUTTON_STATE_DEFAULT);

            }
			break;
		case 9:
			// DOWN + CENTER
			break;
		case 10:
			// UP + CENTER
			break;
		case 11:
			// CENTER DOWN + CENTER
			break;
		case 12:
		{
			// CENTER RIGHT + CENTER

			break;
		}
		case 13:
		{
			// CENTER UP + CENTER
			break;
		}
		case 14:
			// CENTER LEFT + CENTER
			break;


	}

	return BUTTON_HANDLED;
}

void cAirdog::display_drone_state() {
	if (pmenu_ctrl->isActive() || log_running) {
		return;
	}

    if (current_button_state == BUTTON_STATE_CONFIRM_TAKEOFF){
		pi2c_disp_ctrl->set_symbols(SYMBOL_U, SYMBOL_P, SYMBOL_DOT);
        return;
    }

    if (current_button_state == BUTTON_STATE_CHOOSE_FUNCTION){
		pi2c_disp_ctrl->set_symbols(SYMBOL_0, SYMBOL_H, SYMBOL_DOT);
        return;
    }

	if (!drone_active) {
		pi2c_disp_ctrl->set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
	} else {

        if (airdog_status.system_status == MAV_STATE_ACTIVE) {
            switch(airdog_status.main_mode) {
                case PX4_CUSTOM_MAIN_MODE_LANDED:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_L, SYMBOL_A, SYMBOL_EMPTY);
                    break;
                case PX4_CUSTOM_MAIN_MODE_LOITER:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_L, SYMBOL_0, SYMBOL_1);
                    break;
                case PX4_CUSTOM_MAIN_MODE_ABS_FOLLOW:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_A, SYMBOL_8, SYMBOL_5);
                    break;
                case PX4_CUSTOM_MAIN_MODE_FOLLOW:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_F, SYMBOL_0, SYMBOL_L);
                    break;
                case PX4_CUSTOM_MAIN_MODE_RTL:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_L, SYMBOL_A, SYMBOL_EMPTY);
                    break;
                case PX4_CUSTOM_MAIN_MODE_MANUAL:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_MINUS, SYMBOL_MINUS, SYMBOL_MINUS);
                    break;
                default:
                    pi2c_disp_ctrl->set_symbols(SYMBOL_0, SYMBOL_F, SYMBOL_F);
                    break;
            }
        }
        else if (airdog_status.system_status == MAV_STATE_STANDBY) {
            pi2c_disp_ctrl->set_symbols(SYMBOL_0, SYMBOL_1, SYMBOL_5);
        } else if (airdog_status.system_status == MAV_STATE_CRITICAL) {
            pi2c_disp_ctrl->set_symbols(SYMBOL_C, SYMBOL_A, SYMBOL_1);
        }
	}

}

void cAirdog::set_current_button_state(BUTTON_STATE new_button_state) {
    current_button_state = new_button_state;
}

void cAirdog::handle_takeoff()
{
	// bool ready_for_takeoff = armed && airdog_status.sub_mode == PX4_CUSTOM_SUB_MODE_AUTO_READY;
	// uint64_t now = hrt_absolute_time();
	// if (takeoff_request_time > 0 && (now - takeoff_request_time) / (1000 * 1000) > 1 && ready_for_takeoff) {
	// 	rtl_triggered_from_battery = false;
	// 	// send_set_state(NAV_STATE_TAKEOFF, MOVE_NONE);
	// 	takeoff_request_time = now;
	// } else if (takeoff_requested && ready_for_takeoff) {
	// 	rtl_triggered_from_battery = false;
	// 	// send_set_state(NAV_STATE_TAKEOFF, MOVE_NONE);
	// 	takeoff_requested = false;
	// 	takeoff_request_time = now;
	// } else if (takeoff_request_time > 0 && airdog_status.sub_mode == PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF) {
	// 	takeoff_request_time = 0;
	// }
}

void cAirdog::display_discharged_mah() {
	if(airdog_status.discharged_mah > 0) {
		pi2c_disp_ctrl->set_symbols_from_int(((int)airdog_status.discharged_mah) / 10);
	} else {
		pi2c_disp_ctrl->set_symbols(SYMBOL_A, SYMBOL_H, SYMBOL_E); //mAH Error
	}
	sleep(1);
}

static void airdog_cycle(void *arg) {
	((cAirdog*)arg)->cycle();
}

static void airdog_start(void *arg) {
	((cAirdog*)arg)->start();
}

static bool airdog_button_pressed_i2c(void *arg, uint8_t button, hrt_abstime time) {
	return ((cAirdog*)arg)->button_pressed_i2c(button, time);
}

static bool airdog_button_clicked_i2c(void *arg, uint8_t button, bool long_press) {
	return ((cAirdog*)arg)->button_clicked_i2c(button, long_press);
}

static void airdog_usage(const char *reason)
{
	if (reason)
		warnx("%s\n", reason);
	errx(1, "usage: airdog {start|stop|status} [-p <additional params>]\n\n");
}

int airdog_main(int argc, char *argv[])
{
	int res;
	int buzzer = -1;
	int rgbleds = -1;

	if (argc < 1)
		airdog_usage("missing command");

	if (!strcmp(argv[1], "start")) {
		if(g_pAirdog != nullptr) {
			errx(1, "airdog is already running");
		}

		g_pAirdog = new cAirdog();
		if(g_pAirdog == nullptr) {
			errx(2, "airdog failed to allocate class");
		}

		res = work_queue(LPWORK, &g_pAirdog->work, airdog_start, g_pAirdog, 0);

		if(res != 0) {
			errx(3, "airdog failed to queue work: %d", res);
		} else {
			warnx("airdog button listener starting");
		}
		exit(0);
	}

	if (!strcmp(argv[1], "alert")) {
		warnx("Magnetometer not connected");

        if (buzzer == -1) {
            buzzer = open(TONEALARM_DEVICE_PATH, O_WRONLY);
            rgbleds = open(RGBLED_DEVICE_PATH, 0);
        }
        ioctl(buzzer, TONE_SET_ALARM, TONE_BATTERY_WARNING_FAST_TUNE);
        ioctl(rgbleds, RGBLED_SET_MODE, RGBLED_MODE_BLINK_FAST);
        ioctl(rgbleds, RGBLED_SET_COLOR, RGBLED_COLOR_PURPLE);
        exit(0);
    }

	if (g_pAirdog == nullptr)
		errx(-1, "airdog is not running");

	if (!strcmp(argv[1], "stop")) {
		delete g_pAirdog;
		g_pAirdog = nullptr;
		exit(0);
	}

	if (!strcmp(argv[1], "status")) {
		if(g_pAirdog != nullptr) {
			warnx("\trunning\n");
		} else {
			warnx("\tnot started\n");
		}
		exit(0);
	}

	airdog_usage("unrecognized command");
	exit(4);
}
