/**
 * Path follow mode implementation
 */
#include <nuttx/config.h>

#include <geo/geo.h>
#include <drivers/drv_tone_alarm.h>
#include <fcntl.h>
#include <math.h>
#include <mavlink/mavlink_log.h>
#include <time.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <systemlib/err.h>
#include <uORB/topics/external_trajectory.h>


#include "navigator.h"
#include "path_follow.hpp"

// TODO! Unify velocity/speed naming convention... at least in the scope of a single mode. Duh...
// TODO! Add target signal monitoring

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
		_target_vel_lpf(0.5f),
		_drone_vel_lpf(0.2f),
        _vel_ch_rate_lpf(0.2f),
		_target_velocity(0.5f),
        _drone_velocity(0.0f){
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

	mavlink_log_info(_mavlink_fd, "Follow path activation");

	_mavlink_fd = _navigator->get_mavlink_fd();
	// TODO! This message belongs elsewhere
	mavlink_log_info(_mavlink_fd, "Activated Follow Path");
	warnx("Follow path active! Max _speed control, L1, parameters, memory check, mavlink_fd!");

    zero_setpoint = false;

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
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->previous.valid = false;

    pos_sp_triplet->current.type = SETPOINT_TYPE_VELOCITY;
    pos_sp_triplet->current.lat = target_pos->lat;
    pos_sp_triplet->current.lon = target_pos->lon;
    pos_sp_triplet->current.alt = target_pos->alt + _vertical_offset;
    pos_sp_triplet->current.position_valid = true;
    pos_sp_triplet->current.valid = true;
	pos_sp_triplet->current.abs_velocity_valid = true;
	pos_sp_triplet->current.abs_velocity = 0.0f;

	_navigator->set_position_setpoint_triplet_updated();
    
	target_pos = _navigator->get_target_position();

    _trajectory_distance = 0;
    _last_trajectory_time = target_pos->timestamp;
    _last_point.lat = target_pos->lat;
    _last_point.lon = target_pos->lon;
    _last_point.alt = target_pos->alt;

    if (!_saved_trajectory.add(_last_point, true)) {
        mavlink_log_critical(_mavlink_fd, "Trajectory overflow!");
        warnx("Trajectory overflow!");
    }
}
void PathFollow::on_active() {
	if (!_inited) {
		return; // Wait for the Loiter mode to take over, but avoid pausing main navigator thread
	}

    hrt_abstime t = hrt_absolute_time();
    _dt = _t_prev != 0 ? (t - _t_prev) : 0.0f;
    _t_prev = t;

    _vel_ch_rate_lpf.reset(0.0f);

	// Execute command if received
	if ( update_vehicle_command() ) {
		execute_vehicle_command();
	}

	update_saved_trajectory();
	update_target_velocity();
    update_drone_velocity();

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

    if (zero_setpoint == false) {
        if (!_has_valid_setpoint) {
        // Waiting for the first trajectory point.
            if (check_point_safe()) {
                _has_valid_setpoint = _saved_trajectory.pop(_actual_point);
                if (_has_valid_setpoint) {

                    //mavlink_log_info(_mavlink_fd, "Set first setpoint.");

                    global_pos = _navigator->get_global_position();
                    pos_sp_triplet->previous.type = SETPOINT_TYPE_VELOCITY;
                    pos_sp_triplet->previous.lat = global_pos->lat;
                    pos_sp_triplet->previous.lon = global_pos->lon;
                    pos_sp_triplet->previous.alt = global_pos->alt;
                    pos_sp_triplet->previous.position_valid = true;
                    pos_sp_triplet->previous.valid = true;
                    update_setpoint(_actual_point, pos_sp_triplet->current);
                    
                    /* mc_pos_control is not yet using next sp for velocity sp
                    if (_saved_trajectory.peek(0, _future_point)) {
                        update_setpoint(_future_point, pos_sp_triplet->next);
                    }
                    else {
                        pos_sp_triplet->next.valid = false;
                    }
                    */
                }
            } else {

                //mavlink_log_info(_mavlink_fd, "Set target position setpoint. No trajectory points. ");

                pos_sp_triplet->current.type = SETPOINT_TYPE_VELOCITY;
                pos_sp_triplet->current.lat = target_pos->lat;
                pos_sp_triplet->current.lon = target_pos->lon;
                pos_sp_triplet->current.alt = target_pos->alt + _vertical_offset;
                pos_sp_triplet->current.position_valid = true;
                pos_sp_triplet->current.valid = true;

            }
        }
        else {
        // Flying trough collected points
            

            if (check_current_trajectory_point_passed(_parameters.pafol_acc_rad)) {

                // We've passed the point, we need a new one
                _has_valid_setpoint = _saved_trajectory.pop(_actual_point);
                if (_has_valid_setpoint) {

                    //mavlink_log_info(_mavlink_fd, "Setpoint reached, set next setpoint.");
                    // Distance between reached point and the next stops being trajectory distance
                    // and becomes "drone to setpoint" distance
                    _trajectory_distance -= get_distance_to_next_waypoint(pos_sp_triplet->current.lat,
                        pos_sp_triplet->current.lon, _actual_point.lat, _actual_point.lon);

                    // Having both previous and next waypoint allows L1 algorithm
                    memcpy(&(pos_sp_triplet->previous),&(pos_sp_triplet->current),sizeof(position_setpoint_s));
                    // Trying to prevent L1 algorithm
                    // pos_sp_triplet->previous.valid = false;

                    //mavlink_log_info(_mavlink_fd, "Trajectory point passed. Take next point.");

                    update_setpoint(_actual_point, pos_sp_triplet->current);

                    current_point_passed = false;

                    /* mc_pos_control is not yet using next sp for velocity sp
                    if (_saved_trajectory.peek(0, _future_point)) {
                        update_setpoint(_future_point, pos_sp_triplet->next);
                    }
                    else {
                        pos_sp_triplet->next.valid = false;
                    }
                    */
                }
                else {

                    //mavlink_log_info(_mavlink_fd, "Trajectory point passed. No new points. Follow target. ");

                    pos_sp_triplet->current.type = SETPOINT_TYPE_VELOCITY;
                    pos_sp_triplet->current.lat = target_pos->lat;
                    pos_sp_triplet->current.lon = target_pos->lon;
                    pos_sp_triplet->current.alt = target_pos->alt + _vertical_offset;
                    pos_sp_triplet->current.position_valid = true;
                    pos_sp_triplet->current.valid = true;
                    _trajectory_distance = 0;
                }
            } else {

            }
        }
    }

    _desired_speed = calculate_desired_velocity(calculate_current_distance() - _ok_distance);

    if (zero_setpoint == true){

        if (_desired_speed > 1e-6f) {
            zero_setpoint = false;
            pos_sp_triplet = last_moving_sp_triplet;
            //mavlink_log_critical(_mavlink_fd, "Zero point off.");
        }
    }
    else if (zero_setpoint == false) {

        pos_sp_triplet->current.abs_velocity = _desired_speed;
        pos_sp_triplet->current.abs_velocity_valid = true;

        if (_desired_speed < 1e-6f) {

            //mavlink_log_critical(_mavlink_fd, "Zero point on.");

            zero_setpoint = true;
            last_moving_sp_triplet = pos_sp_triplet;

            global_pos = _navigator->get_global_position();

            pos_sp_triplet->next.valid = false;
            pos_sp_triplet->current.valid = true;
            pos_sp_triplet->previous.valid = false;
            pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
            pos_sp_triplet->current.lat = global_pos->lat;
            pos_sp_triplet->current.lon = global_pos->lon;
        
        }
    }


    _navigator->set_position_setpoint_triplet_updated();

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

	//mavlink_log_info(_mavlink_fd, "New point %.8f %.8f", (double)desired_point.lat, (double)desired_point.lon);

	destination.type = SETPOINT_TYPE_VELOCITY;
	destination.lat = desired_point.lat;
	destination.lon = desired_point.lon;
	destination.alt = desired_point.alt + _vertical_offset;
	destination.position_valid = true;
	destination.valid = true;
}

void PathFollow::update_target_velocity() {

	target_pos = _navigator->get_target_position();

    math::Vector<3> target_velocity_vect;

    target_velocity_vect(0) = target_pos->vel_n;
    target_velocity_vect(1) = target_pos->vel_e;
    target_velocity_vect(2) = 0.0f;

    _target_velocity = target_velocity_vect.length();
    math::Vector<3> target_velocity_f_vect = _target_vel_lpf.apply(target_pos->timestamp, target_velocity_vect);
    _target_velocity_f = target_velocity_f_vect.length();
}

void PathFollow::update_drone_velocity() {

	global_pos = _navigator->get_global_position();

    math::Vector<3> global_velocity;

    global_velocity(0) = global_pos->vel_n;
    global_velocity(1) = global_pos->vel_e;
    global_velocity(2) = 0.0f;

    _drone_velocity = global_velocity.length();
    math::Vector<3> drone_velocity_f_vect = _drone_vel_lpf.apply(global_pos->timestamp, global_velocity);
    _drone_velocity_f = drone_velocity_f_vect.length();

}

void PathFollow::update_min_max_dist() {

    _ok_distance = 12.0f;

}

float PathFollow::calculate_desired_velocity(float dst_to_ok) {



    // calculate drone speed change rate and filter it
    if (_global_pos_timestamp_last != global_pos->timestamp){

        float ch_rate_dt = global_pos->timestamp - _global_pos_timestamp_last;

        if (_global_pos_timestamp_last == 0) {

            ch_rate_dt = 0.0f; 
            _vel_ch_rate_f = 0.0f;
            _vel_ch_rate = 0.0f;

        } else {

            ch_rate_dt /= 1000000.0f;

            float delta_vel = _drone_velocity_f - _last_drone_velocity_f;

            _vel_ch_rate = delta_vel / ch_rate_dt;

            _vel_ch_rate_f = _vel_ch_rate_lpf.apply(global_pos->timestamp, _vel_ch_rate);

        }
        
        _global_pos_timestamp_last = global_pos->timestamp;

        _last_drone_velocity_f = _drone_velocity_f ;
        _last_drone_velocity = _drone_velocity;

    }


    hrt_abstime t = hrt_absolute_time();
    float calc_vel_dt = _calc_vel_t_prev != 0 ? (t - _calc_vel_t_prev) : 0.0f;
    _calc_vel_t_prev = t;

    calc_vel_dt/= 1000000.0f;

    //float vel_err_coif = 0.5f;
    float vel_err_coif = _parameters.pafol_vel_err_coif;
    //float vel_err_growth_power = 1.2f;
    float vel_err_growth_power = _parameters.pafol_vel_err_growth_power;
    float max_vel_err;

    if (dst_to_ok >= 0.0f){
        max_vel_err = (float)pow(dst_to_ok, vel_err_growth_power ) * vel_err_coif;

        if (max_vel_err > _parameters.mpc_max_speed - _target_velocity)
            max_vel_err = _parameters.mpc_max_speed - _target_velocity;

    } else {
        max_vel_err = (float)pow(-dst_to_ok, 2.0f) * vel_err_coif;
    }
    

    float reaction_time_to_close = 0.5f; // time in seconds when we increase speed from _target_velocity till _target_velocity + max_vel_err
    float fraction_to_close = calc_vel_dt / reaction_time_to_close; // full increase will happen in reaction_time time, so we calculate how much we need to increase in dt time

    float reaction_time = _parameters.pafol_vel_reaction_time; // time in seconds when we increase speed from _target_velocity till _target_velocity + max_vel_err
    float fraction = calc_vel_dt / reaction_time; // full increase will happen in reaction_time time, so we calculate how much we need to increase in dt time


    float sp_velocity = pos_sp_triplet->current.abs_velocity;

    float vel_new = 0.0f;

    if (dst_to_ok > 0.0f) {
        if (_vel_ch_rate_f < -0.5f  && sp_velocity > 0.0f){ // negative acceleration - we need to reset sp velocity so we can grow it from there
            vel_new = sp_velocity - fraction * (sp_velocity - _drone_velocity_f); // when velocity of drone is decreasing decrease setpoint velocity, so they are synced
            if (vel_new < _drone_velocity)
                vel_new = _drone_velocity;
        } else {
            vel_new = sp_velocity + fraction * max_vel_err;  // while speed is increasing we can smoothly increase velocity if setoibt
        }
    } else {
        vel_new = sp_velocity - fraction_to_close * max_vel_err; // Do the same calculation also when we are to close// maybe we should make this more smooth
    }

    if (vel_new > _target_velocity_f + max_vel_err)  
        vel_new = _target_velocity_f + max_vel_err;

    if (vel_new < 0.0f)
        vel_new = 0.0f;
    

    dd_log.log(0,(double)_target_velocity);
    dd_log.log(1,(double)_drone_velocity);
    dd_log.log(2,(double)sp_velocity);
    dd_log.log(3,(double)dst_to_ok);
    dd_log.log(4,(double)_vel_ch_rate_f);
    dd_log.log(5,(double)_trajectory_distance);
    dd_log.log(6,(double)zero_setpoint);

	if (vel_new > _parameters.mpc_max_speed) 
        vel_new = _parameters.mpc_max_speed;

    //int traj_setpoints = _saved_trajectory.get_value_count();
	// mavlink_log_info(_mavlink_fd, "Calculate desired velocity %.3f %.3f %.3f ", (double)dst_to_ok, (double)vel_new, (double)traj_setpoints);
	//mavlink_log_info(_mavlink_fd, "Dst_to_ok %.3f", (double)dst_to_ok );
    

	return vel_new;

}

float PathFollow::calculate_current_distance() {

    global_pos = _navigator->get_global_position();
    target_pos = _navigator->get_target_position();

    float dst_line = get_distance_to_next_waypoint(global_pos->lat, global_pos->lon, target_pos->lat, target_pos->lon);

	float dst_traj = _trajectory_distance;

    if (_saved_trajectory.get_value_count() > 0){

        dst_traj += get_distance_to_next_waypoint(global_pos->lat, global_pos->lon, _actual_point.lat, _actual_point.lon);
        dst_traj += get_distance_to_next_waypoint(_last_point.lat, _last_point.lon, target_pos->lat, target_pos->lon);

    } else {
        dst_traj = 0.0f;
    }

	return dst_traj > dst_line ? dst_traj : dst_line;
}

bool PathFollow::check_point_safe() {

	buffer_point_s proposed_point;
	target_pos = _navigator->get_target_position();

	if (!_saved_trajectory.peek(0, proposed_point)) {
		return false;
	}
    return true;

}

bool PathFollow::check_current_trajectory_point_passed(float acceptance_dst) {

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	global_pos = _navigator->get_global_position();

	struct map_projection_reference_s _ref_pos;
    map_projection_init(&_ref_pos, global_pos->lat, global_pos->lon);

    math::Vector<2> dst_xy; 
    
    map_projection_project(&_ref_pos,
				pos_sp_triplet->current.lat, pos_sp_triplet->current.lon,
				&dst_xy.data[0], &dst_xy.data[1]);

    math::Vector<2> vel_xy(global_pos->vel_n, global_pos->vel_e);  

    double dst_xy_len = dst_xy.length();
    double dot_product = vel_xy(0) * dst_xy(1) - vel_xy(1) * dst_xy(0);
    double h = dst_xy_len / dot_product;
    double dst_to_line;

    if (h + 1e-6 > dst_xy_len) 
        dst_to_line = 0.0f;
    else 
        dst_to_line = sqrt(dst_xy_len*dst_xy_len - h*h);

	//mavlink_log_info(_mavlink_fd, "Dst to line %.5f", dst_to_line);

    if (acceptance_dst >= (float)dst_to_line && (float)dst_xy_len <= 3 * acceptance_dst )
        return true;
    else 
        return false;
}


