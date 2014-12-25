/****************************************************************************
 *
 *   Copyright (c) 2013, 2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/**
 * @file navigator_main.cpp
 *
 * Handles mission items, geo fencing and failsafe navigation behavior.
 * Published the position setpoint triplet for the position controller.
 *
 * @author Lorenz Meier <lm@inf.ethz.ch>
 * @author Jean Cyr <jean.m.cyr@gmail.com>
 * @author Julian Oes <julian@oes.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 * @author Thomas Gubler <thomasgubler@gmail.com>
 */

#include <nuttx/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <drivers/device/device.h>
#include <drivers/drv_hrt.h>
#include <arch/board/board.h>

#include <uORB/uORB.h>
#include <uORB/topics/home_position.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/mission.h>
#include <uORB/topics/fence.h>
#include <uORB/topics/navigation_capabilities.h>
#include <drivers/drv_baro.h>

#include <systemlib/err.h>
#include <systemlib/systemlib.h>
#include <geo/geo.h>
#include <dataman/dataman.h>
#include <mathlib/mathlib.h>
#include <mavlink/mavlink_log.h>

#include "navigator.h"

/**
 * navigator app start / stop handling function
 *
 * @ingroup apps
 */
extern "C" __EXPORT int navigator_main(int argc, char *argv[]);

#define GEOFENCE_CHECK_INTERVAL 200000

static bool is_empty(double array[3]);

namespace navigator
{

Navigator	*g_navigator;
}

Navigator::Navigator() :
	SuperBlock(NULL, "NAV"),
	_task_should_exit(false),
	_navigator_task(-1),
	_mavlink_fd(-1),
	_global_pos_sub(-1),
	_gps_pos_sub(-1),
	_home_pos_sub(-1),
	_vstatus_sub(-1),
	_capabilities_sub(-1),
	_control_mode_sub(-1),
	_onboard_mission_sub(-1),
	_offboard_mission_sub(-1),
	_param_update_sub(-1),
	_vcommand_sub(-1),
	_target_pos_sub(-1),
	_target_trajectory_sub(-1),
    _first_leash_point(),
    _last_leash_point(),
	_pos_sp_triplet_pub(-1),
	_mission_result_pub(-1),
	_att_sp_pub(-1),
	_commander_request_pub(-1),
	_vstatus{},
	_control_mode{},
	_global_pos{},
	_gps_pos{},
	_sensor_combined{},
	_home_pos{},
	_mission_item{},
	_nav_caps{},
	_pos_sp_triplet{},
	_mission_result{},
	_att_sp{},
	_target_trajectory{},
	_mission_item_valid(false),
	_loop_perf(perf_alloc(PC_ELAPSED, "navigator")),
	_geofence{},
	_geofence_violation_warning_sent(false),
	_inside_fence(true),
	_navigation_mode(nullptr),
	_mission(this, "MIS"),
	_loiter(this, "LOI"),
	_rtl(this, "RTL"),
	_rcLoss(this, "RCL"),
	_dataLinkLoss(this, "DLL"),
	_engineFailure(this, "EF"),
	_gpsFailure(this, "GPSF"),
	_abs_follow(this, "FOL"),
	_path_follow(this, "PAT"),
    _cable_path(this, "CBL"),
    _land(this, "LND"),
	_can_loiter_at_sp(false),
	_pos_sp_triplet_updated(false),
	_commander_request_updated(false),
	_param_loiter_radius(this, "LOITER_RAD"),
	_param_acceptance_radius(this, "ACC_RAD"),
	_param_datalinkloss_obc(this, "DLL_OBC"),
	_param_rcloss_obc(this, "RCL_OBC")
{
	/* Create a list of our possible navigation types */
	_navigation_mode_array[0] = &_mission;
	_navigation_mode_array[1] = &_loiter;
	_navigation_mode_array[2] = &_rtl;
	_navigation_mode_array[3] = &_dataLinkLoss;
	_navigation_mode_array[4] = &_engineFailure;
	_navigation_mode_array[5] = &_gpsFailure;
	_navigation_mode_array[6] = &_rcLoss;
	_navigation_mode_array[7] = &_abs_follow;
	_navigation_mode_array[8] = &_path_follow;
	_navigation_mode_array[9] = &_land;
    _navigation_mode_array[10] = &_cable_path;

	updateParams();

	_commander_request.request_type = OTHER;
}

Navigator::~Navigator()
{
	if (_navigator_task != -1) {

		/* task wakes up every 100ms or so at the longest */
		_task_should_exit = true;

		/* wait for a second for the task to quit at our request */
		unsigned i = 0;

		do {
			/* wait 20ms */
			usleep(20000);

			/* if we have given up, kill it */
			if (++i > 50) {
				task_delete(_navigator_task);
				break;
			}
		} while (_navigator_task != -1);
	}

	navigator::g_navigator = nullptr;
}

void
Navigator::global_position_update()
{
	orb_copy(ORB_ID(vehicle_global_position), _global_pos_sub, &_global_pos);
}

void
Navigator::gps_position_update()
{
	orb_copy(ORB_ID(vehicle_gps_position), _gps_pos_sub, &_gps_pos);
}

void
Navigator::sensor_combined_update()
{
	orb_copy(ORB_ID(sensor_combined), _sensor_combined_sub, &_sensor_combined);
}

void
Navigator::home_position_update()
{
	orb_copy(ORB_ID(home_position), _home_pos_sub, &_home_pos);
}

void
Navigator::navigation_capabilities_update()
{
	orb_copy(ORB_ID(navigation_capabilities), _capabilities_sub, &_nav_caps);
}

void
Navigator::vehicle_status_update()
{
	if (orb_copy(ORB_ID(vehicle_status), _vstatus_sub, &_vstatus) != OK) {
		/* in case the commander is not be running */
		_vstatus.arming_state = ARMING_STATE_STANDBY;
	}
}

void
Navigator::vehicle_control_mode_update()
{
	if (orb_copy(ORB_ID(vehicle_control_mode), _control_mode_sub, &_control_mode) != OK) {
		/* in case the commander is not be running */
		_control_mode.flag_control_auto_enabled = false;
		_control_mode.flag_armed = false;
	}
}

void
Navigator::params_update()
{
	parameter_update_s param_update;
	orb_copy(ORB_ID(parameter_update), _param_update_sub, &param_update);
}

void
Navigator::target_position_update()
{
	orb_copy(ORB_ID(target_global_position), _target_pos_sub, &_target_pos);
}

void
Navigator::target_trajectory_update()
{
	orb_copy(ORB_ID(external_trajectory), _target_trajectory_sub, &_target_trajectory);
}

void
Navigator::task_main_trampoline(int argc, char *argv[])
{
	navigator::g_navigator->task_main();
}

bool
Navigator::set_next_path_point(double point[3]) {
    if ( is_empty(_first_leash_point) ) {
        fprintf(stderr, "[nav] Got first point on path\n");
        _first_leash_point[0] = point[0];
        _first_leash_point[1] = point[1];
        _first_leash_point[2] = point[2];
        return true;
    } else if ( is_empty(_last_leash_point) ) {
        fprintf(stderr, "[nav] Got second point on path\n");
        _last_leash_point[0] = point[0];
        _last_leash_point[1] = point[1];
        _last_leash_point[2] = point[2];
        return true;
    } else {
        fprintf(stderr, "[nav] Not setting point, already had 2\n");
        return false;
    }
}

bool
Navigator::get_path_points(int point_num, double to_point[2]) {
    switch (point_num) {
        case 0:
            if (is_empty(_first_leash_point))
                return false;
            to_point[0] = _first_leash_point[0];
            to_point[1] = _first_leash_point[1];
            //to_point[2] = _first_leash_point[2];
            break;
        case 1:
            if (is_empty(_last_leash_point))
                return false;
            to_point[0] = _last_leash_point[0];
            to_point[1] = _last_leash_point[1];
            //to_point[2] = _last_leash_point[2];
            break;
        default:
            return false;
    }
    return true;
}

void
Navigator::clear_path_points() {
    _first_leash_point[0] = 0.0;
    _first_leash_point[1] = 0.0;
    _first_leash_point[2] = 0.0;
    _last_leash_point[0] = 0.0;
    _last_leash_point[1] = 0.0;
    _last_leash_point[2] = 0.0;
    fprintf(stderr, "[nav] Path points cleared - set to 0.0\n");
}

void
Navigator::task_main()
{
	/* inform about start */
	warnx("Initializing..");

	_mavlink_fd = open(MAVLINK_LOG_DEVICE, 0);
	_geofence.setMavlinkFd(_mavlink_fd);

	/* Try to load the geofence:
	 * if /fs/microsd/etc/geofence.txt load from this file
	 * else clear geofence data in datamanager */
	struct stat buffer;

	if (stat(GEOFENCE_FILENAME, &buffer) == 0) {
		warnx("Try to load geofence.txt");
		_geofence.loadFromFile(GEOFENCE_FILENAME);

	} else {
		mavlink_log_critical(_mavlink_fd, "#audio: No geofence file");
		if (_geofence.clearDm() > 0)
			warnx("Geofence cleared");
		else
			warnx("Could not clear geofence");
	}

	/* do subscriptions */
	_global_pos_sub = orb_subscribe(ORB_ID(vehicle_global_position));
	_gps_pos_sub = orb_subscribe(ORB_ID(vehicle_gps_position));
	_sensor_combined_sub = orb_subscribe(ORB_ID(sensor_combined));
	_capabilities_sub = orb_subscribe(ORB_ID(navigation_capabilities));
	_vstatus_sub = orb_subscribe(ORB_ID(vehicle_status));
	_control_mode_sub = orb_subscribe(ORB_ID(vehicle_control_mode));
	_home_pos_sub = orb_subscribe(ORB_ID(home_position));
	_onboard_mission_sub = orb_subscribe(ORB_ID(onboard_mission));
	_offboard_mission_sub = orb_subscribe(ORB_ID(offboard_mission));
	_param_update_sub = orb_subscribe(ORB_ID(parameter_update));
	_vcommand_sub = orb_subscribe(ORB_ID(vehicle_command));
	_target_pos_sub = orb_subscribe(ORB_ID(target_global_position));
	_target_trajectory_sub = orb_subscribe(ORB_ID(external_trajectory));

	/* copy all topics first time */
	vehicle_status_update();
	vehicle_control_mode_update();
	global_position_update();
	gps_position_update();
	sensor_combined_update();
	home_position_update();
	navigation_capabilities_update();
	params_update();
	target_position_update();
	target_trajectory_update();

	/* Init the path_follow mode to allocate trajectory buffer */
	if (_path_follow.init()) {
		warnx("Successfully inited follow path mode");
	}
	else {
		// TODO! Consider exiting at this point or deny mode switch
		warnx("Failed to init follow path mode");
	}

	/* rate limit position updates to 50 Hz */
	orb_set_interval(_global_pos_sub, 20);

	hrt_abstime mavlink_open_time = 0;
	const hrt_abstime mavlink_open_interval = 500000;

	/* wakeup source(s) */
	struct pollfd fds[10];

	/* Setup of loop */
	fds[0].fd = _global_pos_sub;
	fds[0].events = POLLIN;
	fds[1].fd = _home_pos_sub;
	fds[1].events = POLLIN;
	fds[2].fd = _capabilities_sub;
	fds[2].events = POLLIN;
	fds[3].fd = _vstatus_sub;
	fds[3].events = POLLIN;
	fds[4].fd = _control_mode_sub;
	fds[4].events = POLLIN;
	fds[5].fd = _param_update_sub;
	fds[5].events = POLLIN;
	fds[6].fd = _sensor_combined_sub;
	fds[6].events = POLLIN;
	fds[7].fd = _gps_pos_sub;
	fds[7].events = POLLIN;
	fds[8].fd = _target_pos_sub;
	fds[8].events = POLLIN;
	fds[9].fd = _target_trajectory_sub;
	fds[9].events = POLLIN;

	int _poll_cnt = sizeof(fds)/sizeof(fds[0]);
	for (int i = 0; i < _poll_cnt; ++i) {
		orb_set_interval(fds[i].fd, 20);
	}

	while (!_task_should_exit) {

		/* wait for up to 100ms for data */
		int pret = poll(&fds[0], (sizeof(fds) / sizeof(fds[0])), 100);

		if (pret == 0) {
			/* timed out - periodic check for _task_should_exit, etc. */
			continue;

		} else if (pret < 0) {
			/* this is undesirable but not much we can do - might want to flag unhappy status */
			warn("poll error %d, %d", pret, errno);
			continue;
		}

		perf_begin(_loop_perf);

		if (_mavlink_fd < 0 && hrt_absolute_time() > mavlink_open_time) {
			/* try to reopen the mavlink log device with specified interval */
			mavlink_open_time = hrt_abstime() + mavlink_open_interval;
			_mavlink_fd = open(MAVLINK_LOG_DEVICE, 0);
		}

		/* target trajectory updated */
		if (fds[9].revents & POLLIN) {
			target_trajectory_update();
		}

		/* target position updated */
		if (fds[8].revents & POLLIN) {
			target_position_update();
		}

		static bool have_geofence_position_data = false;

		/* gps updated */
		if (fds[7].revents & POLLIN) {
			gps_position_update();
			if (_geofence.getSource() == Geofence::GF_SOURCE_GPS) {
				have_geofence_position_data = true;
			}
		}

		/* sensors combined updated */
		if (fds[6].revents & POLLIN) {
			sensor_combined_update();
		}

		bool parameters_updated = false;

		/* parameters updated */
		if (fds[5].revents & POLLIN) {
			params_update();
			updateParams();
			parameters_updated = true;
		}

		/* vehicle control mode updated */
		if (fds[4].revents & POLLIN) {
			vehicle_control_mode_update();
		}

		/* vehicle status updated */
		if (fds[3].revents & POLLIN) {
			vehicle_status_update();
		}

		/* navigation capabilities updated */
		if (fds[2].revents & POLLIN) {
			navigation_capabilities_update();
		}

		/* home position updated */
		if (fds[1].revents & POLLIN) {
			home_position_update();
		}

		/* global position updated */
		if (fds[0].revents & POLLIN) {
			global_position_update();
			if (_geofence.getSource() == Geofence::GF_SOURCE_GLOBALPOS) {
				have_geofence_position_data = true;
			}
		}

		/* Check geofence violation */
		static hrt_abstime last_geofence_check = 0;
		if (have_geofence_position_data && hrt_elapsed_time(&last_geofence_check) > GEOFENCE_CHECK_INTERVAL) {
			bool inside = _geofence.inside(_global_pos, _gps_pos, _sensor_combined.baro_alt_meter);
			last_geofence_check = hrt_absolute_time();
			have_geofence_position_data = false;
			if (!inside) {
				/* inform other apps via the mission result */
				_mission_result.geofence_violated = true;
				publish_mission_result();

				/* Issue a warning about the geofence violation once */
				if (!_geofence_violation_warning_sent) {
					mavlink_log_critical(_mavlink_fd, "#audio: Geofence violation");
					_geofence_violation_warning_sent = true;
				}
			} else {
				/* inform other apps via the mission result */
				_mission_result.geofence_violated = false;
				publish_mission_result();
				/* Reset the _geofence_violation_warning_sent field */
				_geofence_violation_warning_sent = false;
			}
		}

		/* Do stuff according to navigation state set by commander */
		switch (_vstatus.nav_state) {
			case NAVIGATION_STATE_MANUAL:
			case NAVIGATION_STATE_ACRO:
			case NAVIGATION_STATE_ALTCTL:
			case NAVIGATION_STATE_POSCTL:
			case NAVIGATION_STATE_TERMINATION:
			case NAVIGATION_STATE_OFFBOARD:
			case NAVIGATION_STATE_FOLLOW:
				_navigation_mode = nullptr;
				_can_loiter_at_sp = false;
				break;
			case NAVIGATION_STATE_LAND:
                _navigation_mode = &_land;
                break;
			case NAVIGATION_STATE_DESCEND:
			//TODO: [INE] create custom mode for safe descending without valid local position
                _navigation_mode = &_land;
                break;
			case NAVIGATION_STATE_AUTO_MISSION:
				_navigation_mode = &_mission;
				break;
			case NAVIGATION_STATE_LOITER:
				_navigation_mode = &_loiter;
				break;
			case NAVIGATION_STATE_AUTO_RCRECOVER:
				if (_param_rcloss_obc.get() != 0) {
					_navigation_mode = &_rcLoss;
				} else {
					_navigation_mode = &_rtl;
				}
				break;
			case NAVIGATION_STATE_RTL:
					_navigation_mode = &_rtl;
				break;
			case NAVIGATION_STATE_AUTO_RTGS:
				/* Use complex data link loss mode only when enabled via param
				* otherwise use rtl */
				if (_param_datalinkloss_obc.get() != 0) {
					_navigation_mode = &_dataLinkLoss;
				} else {
					_navigation_mode = &_rtl;
				}
				break;
			case NAVIGATION_STATE_AUTO_LANDENGFAIL:
				_navigation_mode = &_engineFailure;
				break;
			case NAVIGATION_STATE_ABS_FOLLOW:
				_navigation_mode = &_abs_follow;
				break;
            case NAVIGATION_STATE_CABLE_PARK:
                // Switch to cable park mode only if there is enough points
                if (       !is_empty(_first_leash_point)
                        && !is_empty(_last_leash_point) ){
                    _navigation_mode = &_cable_path;
                }
                break;
			case NAVIGATION_STATE_AUTO_PATH_FOLLOW:
				_navigation_mode = &_path_follow;
				break;
			case NAVIGATION_STATE_AUTO_LANDGPSFAIL:
				_navigation_mode = &_gpsFailure;
				break;
			default:
				_navigation_mode = nullptr;
				_can_loiter_at_sp = false;
				break;
		}

		/* iterate through navigation modes and set active/inactive for each */
		for(unsigned int i = 0; i < NAVIGATOR_MODE_ARRAY_SIZE; i++) {
			_navigation_mode_array[i]->run(_navigation_mode == _navigation_mode_array[i], parameters_updated);
		}

		/* if nothing is running, set position setpoint triplet invalid */
		if (_navigation_mode == nullptr) {
			// TODO publish empty sp only once
			_pos_sp_triplet.previous.valid = false;
			_pos_sp_triplet.current.valid = false;
			_pos_sp_triplet.next.valid = false;
			_pos_sp_triplet_updated = true;
		}

		if (_pos_sp_triplet_updated) {
			publish_position_setpoint_triplet();
			_pos_sp_triplet_updated = false;
		}

		if (_commander_request_updated) {
			publish_commander_request();
			_commander_request_updated = false;
		}

		perf_end(_loop_perf);
	}
	warnx("exiting.");

	_navigator_task = -1;
	_exit(0);
}

int
Navigator::start()
{
	ASSERT(_navigator_task == -1);

	/* start the task */
	_navigator_task = task_spawn_cmd("navigator",
					 SCHED_DEFAULT,
					 SCHED_PRIORITY_MAX - 5,
					 2500,
					 (main_t)&Navigator::task_main_trampoline,
					 nullptr);

	if (_navigator_task < 0) {
		warn("task start failed");
		return -errno;
	}

	return OK;
}

void
Navigator::status()
{
	/* TODO: add this again */
	// warnx("Global position is %svalid", _global_pos_valid ? "" : "in");

	// if (_global_pos.global_valid) {
	// 	warnx("Longitude %5.5f degrees, latitude %5.5f degrees", _global_pos.lon, _global_pos.lat);
	// 	warnx("Altitude %5.5f meters, altitude above home %5.5f meters",
	// 	      (double)_global_pos.alt, (double)(_global_pos.alt - _home_pos.alt));
	// 	warnx("Ground velocity in m/s, N %5.5f, E %5.5f, D %5.5f",
	// 	      (double)_global_pos.vel_n, (double)_global_pos.vel_e, (double)_global_pos.vel_d);
	// 	warnx("Compass heading in degrees %5.5f", (double)(_global_pos.yaw * M_RAD_TO_DEG_F));
	// }

	if (_geofence.valid()) {
		warnx("Geofence is valid");
		/* TODO: needed? */
//		warnx("Vertex longitude latitude");
//		for (unsigned i = 0; i < _fence.count; i++)
//		warnx("%6u %9.5f %8.5f", i, (double)_fence.vertices[i].lon, (double)_fence.vertices[i].lat);

	} else {
		warnx("Geofence not set (no /etc/geofence.txt on microsd) or not valid");
	}
}

void
Navigator::publish_position_setpoint_triplet()
{
	/* update navigation state */
	_pos_sp_triplet.nav_state = _vstatus.nav_state;

	/* lazily publish the position setpoint triplet only once available */
	if (_pos_sp_triplet_pub > 0) {
		orb_publish(ORB_ID(position_setpoint_triplet), _pos_sp_triplet_pub, &_pos_sp_triplet);

	} else {
		_pos_sp_triplet_pub = orb_advertise(ORB_ID(position_setpoint_triplet), &_pos_sp_triplet);
	}
}

void
Navigator::publish_commander_request()
{
	/* publish commander request only once available */
	if (_commander_request_pub > 0) {
		orb_publish(ORB_ID(commander_request), _commander_request_pub, &_commander_request);

	} else {
		_commander_request_pub = orb_advertise(ORB_ID(commander_request), &_commander_request);
	}
	//clean up commander request fields
	_commander_request.request_type = OTHER;
	_commander_request.camera_mode_changed = false;
}

void Navigator::add_fence_point(int argc, char *argv[])
{
	_geofence.addPoint(argc, argv);
}

void Navigator::load_fence_from_file(const char *filename)
{
	_geofence.loadFromFile(filename);
}

static void usage()
{
	errx(1, "usage: navigator {start|stop|status|fence|fencefile}");
}

int navigator_main(int argc, char *argv[])
{
	if (argc < 2) {
		usage();
	}

	if (!strcmp(argv[1], "start")) {

		if (navigator::g_navigator != nullptr) {
			errx(1, "already running");
		}

		navigator::g_navigator = new Navigator;

		if (navigator::g_navigator == nullptr) {
			errx(1, "alloc failed");
		}

		if (OK != navigator::g_navigator->start()) {
			delete navigator::g_navigator;
			navigator::g_navigator = nullptr;
			err(1, "start failed");
		}

		return 0;
	}

	if (navigator::g_navigator == nullptr)
		errx(1, "not running");

	if (!strcmp(argv[1], "stop")) {
		delete navigator::g_navigator;
		navigator::g_navigator = nullptr;

	} else if (!strcmp(argv[1], "status")) {
		navigator::g_navigator->status();

	} else if (!strcmp(argv[1], "fence")) {
		navigator::g_navigator->add_fence_point(argc - 2, argv + 2);

	} else if (!strcmp(argv[1], "fencefile")) {
		navigator::g_navigator->load_fence_from_file(GEOFENCE_FILENAME);

	} else {
		usage();
	}

	return 0;
}

void
Navigator::publish_mission_result()
{
	/* lazily publish the mission result only once available */
	if (_mission_result_pub > 0) {
		/* publish mission result */
		orb_publish(ORB_ID(mission_result), _mission_result_pub, &_mission_result);

	} else {
		/* advertise and publish */
		_mission_result_pub = orb_advertise(ORB_ID(mission_result), &_mission_result);
	}
	/* reset reached bool */
	_mission_result.reached = false;
	_mission_result.finished = false;
}

void
Navigator::publish_att_sp()
{
	/* lazily publish the attitude sp only once available */
	if (_att_sp_pub > 0) {
		/* publish att sp*/
		orb_publish(ORB_ID(vehicle_attitude_setpoint), _att_sp_pub, &_att_sp);

	} else {
		/* advertise and publish */
		_att_sp_pub = orb_advertise(ORB_ID(vehicle_attitude_setpoint), &_att_sp);
	}
}

static bool is_empty(double array[3]) {
    return (array[0] == 0.0 && array[1] == 0.0 && array[2] == 0.0);
}
