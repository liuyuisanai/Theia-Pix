/**
 * Path follow mode implementation
 */
#include <nuttx/config.h>

#include <geo/geo.h>
#include <drivers/drv_tone_alarm.h>
#include <fcntl.h>
#include <math.h>
#include <mavlink/mavlink_log.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <systemlib/err.h>
#include <uORB/topics/external_trajectory.h>

#include "navigator.h"
#include "path_follow.hpp"

PathFollow::PathFollow(Navigator *navigator, const char *name):
		NavigatorMode(navigator, name),
		_last_trajectory_time(0),
		_saved_trajectory(),
		_trajectory_distance(0.0f),
		_has_valid_setpoint(false),
		_desired_speed(0.0f),
		_min_distance(0.0f),
		_max_distance(0.0f),
		_ok_distance(0.0f),
		_vertical_offset(0.0f),
		_inited(false),
		_target_vel_lpf(1.0f){

}
PathFollow::~PathFollow() {

}
bool PathFollow::init() {
	updateParameters();
	// TODO! consider passing buffer size to the init method to allow retries with different buffer sizes
	_inited = _saved_trajectory.init(_parameters.pafol_buf_size);
	return (_inited);
}
void PathFollow::on_inactive() {
	// TODO! Consider if we want to continue collecting trajectory data while inactive
	// update_saved_trajectory();
}
void PathFollow::on_activation() {
	_mavlink_fd = _navigator->get_mavlink_fd();
	// TODO! This message belongs elsewhere
	mavlink_log_info(_mavlink_fd, "Activated Follow Path");
	warnx("Follow path active! Max speed control, L1, parameters, memory check, mavlink_fd!");

	if (!_inited) {
		mavlink_log_critical(_mavlink_fd, "Follow Path mode wasn't initialized! Aborting...");
		warnx("Follow Path mode wasn't initialized! Aborting...");
		int buzzer = open(TONEALARM_DEVICE_PATH, O_WRONLY);
		ioctl(buzzer, TONE_SET_ALARM, TONE_NOTIFY_NEGATIVE_TUNE);
		close(buzzer);
		commander_request_s *commander_request = _navigator->get_commander_request();
		commander_request->request_type = V_MAIN_STATE_CHANGE;
		commander_request->main_state = MAIN_STATE_LOITER;
		_navigator->set_commander_request_updated();
		return;
	}

	_has_valid_setpoint = false;

	updateParameters();

	// Reset trajectory and distance offsets
	_saved_trajectory.do_empty();
	update_saved_trajectory();
	global_pos = _navigator->get_global_position();
	target_pos = _navigator->get_target_position();
	_ok_distance = get_distance_to_next_waypoint(global_pos->lat, global_pos->lon, target_pos->lat, target_pos->lon);
	if (_ok_distance < _parameters.pafol_ok_dist) {
		_ok_distance = _parameters.pafol_ok_dist;
	}
	update_min_max_dist();
	_vertical_offset = global_pos->alt - target_pos->alt;
	if (_vertical_offset < _parameters.pafol_min_alt_off) {
		_vertical_offset = _parameters.pafol_min_alt_off;
	}

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	pos_sp_triplet->next.valid = false;
	pos_sp_triplet->previous.valid = false;
	// Reset position setpoint to shoot and loiter until we get an acceptable trajectory point
	pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
	pos_sp_triplet->current.lat = global_pos->lat;
	pos_sp_triplet->current.lon = global_pos->lon;
	pos_sp_triplet->current.alt = global_pos->alt;
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->current.position_valid = true;
	pos_sp_triplet->current.abs_velocity_valid = false;
	// point_camera_to_target(&(pos_sp_triplet->current));
	_navigator->set_position_setpoint_triplet_updated();
}
void PathFollow::on_active() {
	if (!_inited) {
		return; // Wait for the Loiter mode to take over, but avoid pausing main navigator thread
	}

	updateParameters();
	bool setpointChanged = false;

	// Execute command if received
	if ( update_vehicle_command() ) {
		execute_vehicle_command();
	}

	update_saved_trajectory();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	if (!_has_valid_setpoint) {
	// Waiting for the first trajectory point. Should not occur during the flight
		// TODO! Temporary safety measure
		if (check_point_safe()) {
			_has_valid_setpoint = _saved_trajectory.pop(_actual_point);
			if (_has_valid_setpoint) {
				global_pos = _navigator->get_global_position();
				// TODO! Probably this belongs in mc_pos
				pos_sp_triplet->previous.type = SETPOINT_TYPE_POSITION;
				pos_sp_triplet->previous.lat = global_pos->lat;
				pos_sp_triplet->previous.lon = global_pos->lon;
				pos_sp_triplet->previous.alt = global_pos->alt;
				pos_sp_triplet->previous.position_valid = true;
				pos_sp_triplet->previous.valid = true;
				update_setpoint(_actual_point, pos_sp_triplet->current);
				if (_saved_trajectory.peek(0, _future_point)) {
					update_setpoint(_future_point, pos_sp_triplet->next);
				}
				else {
					pos_sp_triplet->next.valid = false;
				}
			}
			setpointChanged = true;
		}
	}
	else {
	// Flying trough collected points
		// TODO! This results in weird behavior if altitude difference is preventing the copter from reaching the setpoint
		if (check_current_pos_sp_reached()) {
		// We've reached the actual setpoint
			// TODO! Temporary safety measure
			if (check_point_safe()) {
				_has_valid_setpoint = _saved_trajectory.pop(_actual_point);
				if (_has_valid_setpoint) {
					// Distance between reached point and the next stops being trajectory distance
					// and becomes "drone to setpoint" distance
					_trajectory_distance -= get_distance_to_next_waypoint(pos_sp_triplet->current.lat,
						pos_sp_triplet->current.lon, _actual_point.lat, _actual_point.lon);

					// Having both previous and next waypoint allows L1 algorithm
					memcpy(&(pos_sp_triplet->previous),&(pos_sp_triplet->current),sizeof(position_setpoint_s));
					// Trying to prevent L1 algorithm
					// pos_sp_triplet->previous.valid = false;

					update_setpoint(_actual_point, pos_sp_triplet->current);
					if (_saved_trajectory.peek(0, _future_point)) {
						update_setpoint(_future_point, pos_sp_triplet->next);
					}
					else {
						pos_sp_triplet->next.valid = false;
					}
				}
				else {
					mavlink_log_info(_mavlink_fd, "Follow path queue empty!");
					warnx("Follow path queue empty!");
					// No trajectory points left
					_trajectory_distance = 0;
					pos_sp_triplet->current.valid = false;
				}
			}
			else {
				mavlink_log_critical(_mavlink_fd, "Safety switch!");
				warnx("Safety switch!");
				_trajectory_distance = 0;
				pos_sp_triplet->current.valid = false;
				_has_valid_setpoint = false;
			}
			setpointChanged = true;
		}
	}

	// In any scenario point the camera to target
	// point_camera_to_target(&(pos_sp_triplet->current));

	// If we have a setpoint, update speed
	if (_has_valid_setpoint) {
		// float angle; // angle of current movement, from -pi to pi
		_desired_speed = calculate_desired_speed(calculate_current_distance());
		// warnx("Absolute desired speed: % 9.6f", double(_desired_speed));
		// global_pos = _navigator->get_global_position();
		// angle = get_bearing_to_next_waypoint(global_pos->lat, global_pos->lon, _actual_point.lat, _actual_point.lon);
		// warnx("Would be speed: x = % 9.6f, y = % 9.6f", double(float(cos((double)angle)) * _desired_speed), double(float(sin((double)angle)) * _desired_speed));
		// TODO! Check what x and y axis stand for in this case and if cos and sin are used correctly
		// pos_sp_triplet->current.vx = float(cos((double)angle)) * _desired_speed;
		// pos_sp_triplet->current.vy = float(sin((double)angle)) * _desired_speed;
		pos_sp_triplet->current.abs_velocity = _desired_speed;
		pos_sp_triplet->current.abs_velocity_valid = true;

		setpointChanged = true;
	}
	// TODO! Reconsider. Currently, if speed management is on, the only case that doesn't change setpoint is wait state
	if (setpointChanged) {
		_navigator->set_position_setpoint_triplet_updated();
	}
}
void PathFollow::execute_vehicle_command() {
	if (_vcommand.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		REMOTE_CMD remote_cmd = (REMOTE_CMD)_vcommand.param1;
		switch (remote_cmd) {
			// Switch to loiter
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}
			case REMOTE_CMD_FURTHER:
				_ok_distance += _parameters.pafol_dist_step;
				update_min_max_dist();
				break;
			case REMOTE_CMD_CLOSER: {
				_ok_distance -= _parameters.pafol_dist_step;
				if (_ok_distance < _parameters.pafol_ok_dist) {
					_ok_distance = _parameters.pafol_ok_dist;
				}
				update_min_max_dist();
				break;
			}
			case REMOTE_CMD_UP: {
				_vertical_offset += _parameters.pafol_alt_step;
				break;
			}
			case REMOTE_CMD_DOWN: {
				_vertical_offset -= _parameters.pafol_alt_step;
				if (_vertical_offset < _parameters.pafol_min_alt_off) {
					_vertical_offset = _parameters.pafol_min_alt_off;
				}
			}
		}
	}
}

void PathFollow::update_saved_trajectory() {
	struct external_trajectory_s *target_trajectory = _navigator->get_target_trajectory();
	// Assuming timestamp won't be 0 on first call
	if (_last_trajectory_time != target_trajectory->timestamp && target_trajectory->point_type != 0) {
		if (_saved_trajectory.is_empty()) {
			_trajectory_distance = 0;
		}
		else {
			_trajectory_distance += get_distance_to_next_waypoint(_last_point.lat, _last_point.lon,
					target_trajectory->lat, target_trajectory->lon);
		}
		_last_trajectory_time = target_trajectory->timestamp;
		_last_point.lat = target_trajectory->lat;
		_last_point.lon = target_trajectory->lon;
		_last_point.alt = target_trajectory->alt;
		if (!_saved_trajectory.add(_last_point, true)) {
			mavlink_log_critical(_mavlink_fd, "Trajectory overflow!");
			warnx("Trajectory overflow!");
		}
	}
}

// TODO! Write two separate methods, one that does copying and applying offsets, other - for prev, next, current point magic
void PathFollow::update_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination) {
	destination.type = SETPOINT_TYPE_POSITION;
	destination.lat = desired_point.lat;
	destination.lon = desired_point.lon;
	destination.alt = desired_point.alt + _vertical_offset;
	destination.position_valid = true;
	destination.valid = true;
}

void PathFollow::update_min_max_dist() {
	_min_distance = _ok_distance - _parameters.pafol_min_ok_diff;
	if (_min_distance < _parameters.pafol_safe_dist) {
		_min_distance = _parameters.pafol_safe_dist;
		// TODO! Do we need this check?
		if (_ok_distance <= _min_distance) {
			// Add 1 meter to avoid division by 0 when calculating speed
			_ok_distance = _min_distance + 1.0f;
		}
	}
	// TODO! Add max limit (as in "no signal")
	_max_distance = _ok_distance * _parameters.pafol_ok_max_coef;
	warnx("Distances updated! Ok: %9.6f, Min: %9.6f, Max: %9.6f.", double(_ok_distance), double(_min_distance), double(_max_distance));
}

// TODO! Parameters for speed calculation functions
float PathFollow::calculate_desired_speed(float distance) {
	if (distance <= _min_distance) {
		return 0;
	}
	else if (distance >= _max_distance) {
		return _parameters.mpc_max_speed;
	}
	float res;
	target_pos = _navigator->get_target_position();
	float target_speed = _target_vel_lpf.apply(target_pos->remote_timestamp, sqrtf(target_pos->vel_n * target_pos->vel_n + target_pos->vel_e * target_pos->vel_e));
	if (distance >= _ok_distance) {
		// TODO! Simplify and add precalculated values
		res = target_speed * ((distance - _ok_distance)*(distance - _ok_distance)*4.0f/(_max_distance-_ok_distance)/(_max_distance-_ok_distance)+1.0f);
	}
	else {
		// TODO! Simplify and add precalculated values
		res = target_speed * ((-atanf((distance-2.5f-_min_distance)*(-20.0f)/(_ok_distance-_min_distance)))/M_PI_F+0.5f);
	}
	// warnx("Target speed: %9.6f, desired speed: %9.6f", (double) target_speed, (double) res);
	if (res < 0.0f) return 0.0f;
	if (res > _parameters.mpc_max_speed) return _parameters.mpc_max_speed;
	return res;
}

float PathFollow::calculate_current_distance() {
	float res = _trajectory_distance;
	global_pos = _navigator->get_global_position();
	res += get_distance_to_next_waypoint(global_pos->lat, global_pos->lon, _actual_point.lat, _actual_point.lon);
	// TODO! Consider returning back if res is less than min distance
	// Will produce the speed of 0 if passed to calculate speed
	if (res <= _min_distance) {
		// TODO! This will produce a sudden change in speed when target is far from the last trajectory point, but drone nears it
		return _min_distance;
	}
	target_pos = _navigator->get_target_position();
	res += get_distance_to_next_waypoint(_last_point.lat, _last_point.lon, target_pos->lat, target_pos->lon);
	// warnx("Current distance: %9.6f", (double) res);
	return res;
}

bool PathFollow::check_point_safe() {
	buffer_point_s proposed_point;
	target_pos = _navigator->get_target_position();
	if (!_saved_trajectory.peek(0, proposed_point)) {
		// TODO! Consider returning true to handle "Queue empty" rather than "safety switch" scenario
		return false;
	}
	// Don't even pick a point that is closer than X meters to the target
	return (get_distance_to_next_waypoint(proposed_point.lat, proposed_point.lon, target_pos->lat, target_pos->lon) >= _parameters.pafol_safe_dist);
}
