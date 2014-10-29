/**
 * Path follow mode implementation
 */
#include <nuttx/config.h>

#include <geo/geo.h>
#include <math.h>
#include <mavlink/mavlink_log.h>
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
		_ok_distance(0.0f){

}
PathFollow::~PathFollow() {

}
bool PathFollow::init() {
	// TODO! Parameters
	return (_saved_trajectory.init(500));
}
void PathFollow::on_inactive() {
	// TODO! Consider if we want to continue collecting trajectory data while inactive
	// update_saved_trajectory();
}
void PathFollow::on_activation() {
	warnx("Follow path active! No speed, prev+cur+next points version!");
	_has_valid_setpoint = false;

	updateParameters();
	// TODO! Parameters or other type of initialization
	_min_distance = 5.0f;
	_max_distance = 20.0f;
	_ok_distance = 10.0f;
	// TODO! Consider if we really want to reset the trajectory state
	_saved_trajectory.do_empty();
	update_saved_trajectory();
	global_pos = _navigator->get_global_position();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	pos_sp_triplet->next.valid = false;

	// Reset position setpoint to shoot and loiter until we get an acceptable trajectory point
	pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
	pos_sp_triplet->current.lat = global_pos->lat;
	pos_sp_triplet->current.lon = global_pos->lon;
	pos_sp_triplet->current.alt = global_pos->alt;
	point_camera_to_target(&(pos_sp_triplet->current));
	_navigator->set_position_setpoint_triplet_updated();
}
void PathFollow::on_active() {
	updateParameters();
	update_saved_trajectory();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	if (!_has_valid_setpoint) {
		// Waiting for the first trajectory point. Should not occur during the flight
		_has_valid_setpoint = _saved_trajectory.pop(_actual_point);
		if (_has_valid_setpoint) {
			update_setpoint(_actual_point, pos_sp_triplet->current);
			if (_saved_trajectory.peek(1, _future_point)) {
				update_setpoint(_future_point, pos_sp_triplet->next);
			}
			else {
				pos_sp_triplet->next.valid = false;
			}
		}
	}
	else {
		// TODO! Update the check for moving points and check for Follow Path compatibility
		if (check_current_pos_sp_reached()) {
			// We've reached the actual setpoint
			_has_valid_setpoint = _saved_trajectory.pop(_actual_point);
			if (_has_valid_setpoint) {
				// Distance between reached point and the next stops being trajectory distance
				// and becomes "drone to setpoint" distance
				_trajectory_distance -= get_distance_to_next_waypoint(pos_sp_triplet->current.lat,
					pos_sp_triplet->current.lon, _actual_point.lat, _actual_point.lon);

				// TODO! Is the previous point needed?
				pos_sp_triplet->previous = pos_sp_triplet->current;
				// Trying to prevent L1 algorithm
				// pos_sp_triplet->previous.valid = false;

				update_setpoint(_actual_point, pos_sp_triplet->current);
				if (_saved_trajectory.peek(1, _future_point)) {
					update_setpoint(_future_point, pos_sp_triplet->next);
				}
				else {
					pos_sp_triplet->next.valid = false;
				}
			}
			else {
				mavlink_log_info(_mavlink_fd, "Follow path queue empty!");
				// No trajectory points left
				_trajectory_distance = 0;
			}
		}
	}

	// In any scenario point the camera to target
	point_camera_to_target(&(pos_sp_triplet->current));

	// If we have a setpoint, update speed
	if (_has_valid_setpoint) {
		/* float angle; // angle of current movement, from -pi to pi
		_desired_speed = calculate_desired_speed(calculate_current_distance());
		// warnx("Absolute speed: % 9.6f", double(_desired_speed));
		global_pos = _navigator->get_global_position();
		angle = get_bearing_to_next_waypoint(global_pos->lat, global_pos->lon, _actual_point.lat, _actual_point.lon);
		// warnx("Would be speed: x = % 9.6f, y = % 9.6f", double(float(cos((double)angle)) * _desired_speed), double(float(sin((double)angle)) * _desired_speed));
		// TODO! Check what x and y axis stand for in this case and if cos and sin are used correctly
		pos_sp_triplet->current.vx = float(cos((double)angle)) * _desired_speed;
		pos_sp_triplet->current.vy = float(sin((double)angle)) * _desired_speed;
		pos_sp_triplet->current.velocity_valid = true;*/
	}

	// In any scenario setpoint could have changed
	_navigator->set_position_setpoint_triplet_updated();
}
void PathFollow::execute_vehicle_command() {

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
		}
	}
}

// TODO! Add velocity
// TODO! Write two separate methods, one that does copying and applying offsets, other - for prev, next, current point magic
void PathFollow::update_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination) {
	// TODO! Parameters
	float offset = 5;
	destination.type = SETPOINT_TYPE_POSITION;
	destination.lat = desired_point.lat;
	destination.lon = desired_point.lon;
	destination.alt = desired_point.alt + offset;
	destination.position_valid = true;
}

// TODO! Parameters for speed calculation functions
float PathFollow::calculate_desired_speed(float distance) {
	float max_speed = 11.1111f;
	if (distance <= _min_distance) {
		return 0;
	}
	else if (distance >= _max_distance) {
		// TODO! Max speed
		return max_speed;
	}

	float res;
	target_pos = _navigator->get_target_position();
	float target_speed = float(sqrt(double(target_pos->vel_n * target_pos->vel_n + target_pos->vel_e * target_pos->vel_e)));
	if (distance >= _ok_distance) {
		// TODO! Simplify and add precalculated values
		res = target_speed * ((distance - _ok_distance)*(distance - _ok_distance)*4.0f/(_max_distance-_ok_distance)/(_max_distance-_ok_distance)+1.0f);
	}
	else {
		// TODO! Simplify and add precalculated values
		res = target_speed * ((float(-atan(double(-(distance-2.5f-_min_distance)*20.0f/(_ok_distance-_min_distance))))/M_PI_F)+0.5f);
	}
	if (res < 0.0f) return 0.0f;
	if (res > max_speed) return max_speed;
	return res;
}

float PathFollow::calculate_current_distance() {
	float res = _trajectory_distance;
	global_pos = _navigator->get_global_position();
	res += get_distance_to_next_waypoint(global_pos->lat, global_pos->lon, _actual_point.lat, _actual_point.lon);
	// TODO! Consider returning back if res is less than min distance
	// Will produce the speed of 0 if passed to calculate speed
	if (res <= _min_distance) {
		return _min_distance;
	}
	target_pos = _navigator->get_target_position();
	res += get_distance_to_next_waypoint(_last_point.lat, _last_point.lon, target_pos->lat, target_pos->lon);
	return res;
}
