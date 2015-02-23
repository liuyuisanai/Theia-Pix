/**
 * @file path_follow.cpp
 *
 * Path follow navigator mode implementation 
 *
 * @author Anthony Kenga <anton.k@airdog.vom>
 * @author Martins Frolovs <martins.f@airdog.vom>
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

PathFollow::PathFollow(Navigator *navigator, const char *name):
		NavigatorMode(navigator, name),
		_last_trajectory_time(0),
		_saved_trajectory(),
		_trajectory_distance(0.0f),
		_has_valid_setpoint(false),
		_vertical_offset(0.0f),
		_inited(false),
		_target_vel_lpf(0.2f),
		_drone_vel_lpf(0.2f),
        _vel_ch_rate_lpf(0.2f),
		_target_velocity(0.5f),
        _drone_velocity(0.0f),
		_desired_speed(0.0f),
		_ok_distance(0.0f){
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

	mavlink_log_info(_mavlink_fd, "Follow path activation");

	global_pos = _navigator->get_global_position();
	target_pos = _navigator->get_target_position();
    
    _navigator->set_flag_reset_pfol_offs(false);

    _ok_distance = _parameters.pafol_ok_dist;

    //mavlink_log_info(_mavlink_fd, "%.5f", (double)_ok_distance);

    if (_parameters.follow_rpt_alt == 0) {
        _alt = global_pos->alt;
        _vertical_offset = 0.0f;
    } else {
        _vertical_offset = global_pos->alt - target_pos->alt;
        if (_vertical_offset < _parameters.pafol_min_alt_off) {
            _vertical_offset = _parameters.pafol_min_alt_off;
        }
    }

    zero_setpoint = false;

	if (!_inited) {

		mavlink_log_critical(_mavlink_fd, "Follow Path mode wasn't initialized! Aborting...");

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

	_navigator->invalidate_setpoint_triplet();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	pos_sp_triplet->next.valid = false;
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->previous.valid = false;

    set_target_setpoint(pos_sp_triplet->current);

	pos_sp_triplet->current.abs_velocity = 0.0f;
	pos_sp_triplet->current.abs_velocity_valid = true;
	_navigator->set_position_setpoint_triplet_updated();
    
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
                    put_buffer_point_into_setpoint(_actual_point, pos_sp_triplet->current);
                    
                    /* mc_pos_control is not yet using next sp for control_auto_velvelocity_sp
                    if (_saved_trajectory.peek(0, _future_point)) {
                        put_buffer_point_into_setpoint(_future_point, pos_sp_triplet->next);
                    }
                    else {
                        pos_sp_triplet->next.valid = false;
                    }
                    */
                }
            } else {

                // mavlink_log_info(_mavlink_fd, "Set target position setpoint. No trajectory points. ");
                set_target_setpoint(pos_sp_triplet->current);

            }
        }
        else {
        // Flying trough collected points
            if (check_current_trajectory_point_passed()) {
                //mavlink_log_info(_mavlink_fd, "Point passed ! Get next one ! ");
                // We've passed the point, we need a new one
                _has_valid_setpoint = _saved_trajectory.pop(_actual_point);
                if (_has_valid_setpoint) {

                    // Distance between reached point and the next stops being trajectory distance
                    // and becomes "drone to setpoint" distance
                    _trajectory_distance -= get_distance_to_next_waypoint(pos_sp_triplet->current.lat,
                        pos_sp_triplet->current.lon, _actual_point.lat, _actual_point.lon);

                    // Having both previous and next waypoint allows L1 algorithm
                    memcpy(&(pos_sp_triplet->previous),&(pos_sp_triplet->current),sizeof(position_setpoint_s));
                    // Trying to prevent L1 algorithm
                    pos_sp_triplet->previous.valid = false;

                    put_buffer_point_into_setpoint(_actual_point, pos_sp_triplet->current);

                    /* mc_pos_control is not yet using next sp for velocity sp
                    if (_saved_trajectory.peek(0, _future_point)) {
                        put_buffer_point_into_setpoint(_future_point, pos_sp_triplet->next);
                    }
                    else {
                        pos_sp_triplet->next.valid = false;
                    }
                    */
                }
                else {
                    set_target_setpoint(pos_sp_triplet->current);
                }
            } else {

            }
        }
    }

    _desired_speed = calculate_desired_velocity(calculate_current_distance() - _ok_distance);
    pos_sp_triplet->current.abs_velocity = _desired_speed;
    pos_sp_triplet->current.abs_velocity_valid = true;

  if (zero_setpoint == true){

        if (_desired_speed > 1e-6f) {
            zero_setpoint = false;
            pos_sp_triplet = last_moving_sp_triplet;
            mavlink_log_critical(_mavlink_fd, "Zero point off.");
        }
    }
    else if (zero_setpoint == false) {

        if (_desired_speed < 1e-6f && _drone_velocity < _parameters.pafol_stop_speed ) {

            if (_desired_speed < 0.0f)
                _desired_speed = 0.0f;

            zero_setpoint = true;
            last_moving_sp_triplet = pos_sp_triplet;

            global_pos = _navigator->get_global_position();

            // Will update pos_sp_triplet too as it points to the same location
            _navigator->invalidate_setpoint_triplet();

            pos_sp_triplet->current.type = SETPOINT_TYPE_POSITION;
            pos_sp_triplet->current.lat = global_pos->lat;
            pos_sp_triplet->current.lon = global_pos->lon;

            if (_parameters.follow_rpt_alt == 1) { // in case of alt changes fix altitude to current altitude
                pos_sp_triplet->current.alt = global_pos->alt;
            }
            
            pos_sp_triplet->current.valid = true;
            pos_sp_triplet->current.position_valid = true;
            mavlink_log_critical(_mavlink_fd, "Zero point on.");
            
        }
    }

    _navigator->set_position_setpoint_triplet_updated();

}

void PathFollow::execute_vehicle_command() {
	if (_vcommand.command == VEHICLE_CMD_DO_SET_MODE){

		//uint8_t base_mode = (uint8_t)cmd.param1;
		uint8_t main_mode = (uint8_t)_vcommand.param2;

		if (main_mode == PX4_CUSTOM_MAIN_MODE_RTL) {

			commander_request_s *commander_request = _navigator->get_commander_request();
			commander_request->request_type = V_MAIN_STATE_CHANGE;
			commander_request->main_state = MAIN_STATE_RTL;
			_navigator->set_commander_request_updated();

		}
	}
	if (_vcommand.command == VEHICLE_CMD_NAV_REMOTE_CMD) {
		REMOTE_CMD remote_cmd = (REMOTE_CMD)_vcommand.param1;
		switch (remote_cmd) {
			case  REMOTE_CMD_LAND_DISARM: {

                commander_request_s *commander_request = _navigator->get_commander_request();
                commander_request->request_type = V_MAIN_STATE_CHANGE;
                commander_request->main_state = MAIN_STATE_EMERGENCY_LAND;
                _navigator->set_commander_request_updated();
                break;
            }
			// Switch to loiter
			case REMOTE_CMD_PLAY_PAUSE: {
				commander_request_s *commander_request = _navigator->get_commander_request();
				commander_request->request_type = V_MAIN_STATE_CHANGE;
				commander_request->main_state = MAIN_STATE_LOITER;
				_navigator->set_commander_request_updated();
				break;
			}
			case REMOTE_CMD_FURTHER:
				_ok_distance += _parameters.horizon_button_step;
				break;
			case REMOTE_CMD_CLOSER: {
				_ok_distance -= _parameters.horizon_button_step;
				if (_ok_distance < _parameters.pafol_min_ok_dist) {
					_ok_distance = _parameters.pafol_min_ok_dist;
				}
				break;
			}
			case REMOTE_CMD_UP: {
				_vertical_offset += _parameters.up_button_step;

                pos_sp_triplet->current.alt += _parameters.up_button_step;

				break;
			}
			case REMOTE_CMD_DOWN: {

                float tmp_offset = _vertical_offset;

				_vertical_offset -= _parameters.down_button_step;
				if (_vertical_offset < _parameters.pafol_min_alt_off) {
					_vertical_offset = _parameters.pafol_min_alt_off;
				}

                pos_sp_triplet->current.alt+= (_vertical_offset - tmp_offset); //decrease by difference
			}
		}
	}
}

void PathFollow::update_saved_trajectory() {

	struct external_trajectory_s *target_trajectory = _navigator->get_target_trajectory();

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

void PathFollow::put_buffer_point_into_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination) {

	//mavlink_log_info(_mavlink_fd, "New trajectory point %.8f", (double)desired_point.alt);

	destination.type = SETPOINT_TYPE_VELOCITY;
	destination.lat = desired_point.lat;
	destination.lon = desired_point.lon;
	destination.alt = desired_point.alt + _vertical_offset;

    if (_parameters.follow_rpt_alt == 1) 
        destination.alt = desired_point.alt + _vertical_offset;
    else
        destination.alt = _alt + _vertical_offset;

	destination.position_valid = true;
	destination.valid = true;
}

void PathFollow::set_target_setpoint(position_setpoint_s &destination) {

    //mavlink_log_info(_mavlink_fd, "Set target setpoint.")

    target_pos = _navigator->get_target_position();

    destination.type = SETPOINT_TYPE_VELOCITY;
    destination.lat = target_pos->lat;
    destination.lon = target_pos->lon;

    if (_parameters.follow_rpt_alt == 1) 
        destination.alt = target_pos->alt + _vertical_offset;
    else
        destination.alt = _alt + _vertical_offset;

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

    float vel_err_coif = _parameters.pafol_vel_err_coif;
    float vel_err_growth_power = _parameters.pafol_vel_err_growth_power;
    float max_vel_err;

    float vel_err_growth_power_decr = _parameters.pafol_vel_err_growth_power_decr;

    if (dst_to_ok >= 0.0f){
        max_vel_err = (float)pow(dst_to_ok, vel_err_growth_power ) * vel_err_coif;

        if (max_vel_err > _parameters.mpc_max_speed - _target_velocity)
            max_vel_err = _parameters.mpc_max_speed - _target_velocity;
    } else {
        max_vel_err = (float)pow(-dst_to_ok, vel_err_growth_power_decr);
    }

    float reaction_time_decr = _parameters.pafol_vel_reaction_time_decr; // time in seconds when we increase speed from _target_velocity till _target_velocity + max_vel_err
    float fraction_decr = calc_vel_dt / reaction_time_decr; // full increase will happen in reaction_time time, so we calculate how much we need to increase in dt time

    float reaction_time = _parameters.pafol_vel_reaction_time; // time in seconds when we increase speed from _target_velocity till _target_velocity + max_vel_err
    float fraction = calc_vel_dt / reaction_time; // full increase will happen in reaction_time time, so we calculate how much we need to increase in dt time

    float sp_velocity = pos_sp_triplet->current.abs_velocity;
    float vel_new = 0.0f;

    if (dst_to_ok >= 0.0f) {
        if (_vel_ch_rate_f < -0.5f  && sp_velocity > 0.0f){ // negative acceleration - we need to reset sp velocity so we can grow it from there
            vel_new = sp_velocity - fraction * (sp_velocity - _drone_velocity_f); // when velocity of drone is decreasing decrease setpoint velocity, so they are synced
            if (vel_new < _drone_velocity)
                vel_new = _drone_velocity;
        } else {
            vel_new = sp_velocity + fraction * max_vel_err;  // while speed is increasing we can smoothly increase velocity if setoibt
        }

        if (vel_new > _target_velocity_f + max_vel_err)  
            vel_new = _target_velocity_f + max_vel_err;

    } else {
        vel_new = sp_velocity - fraction_decr * max_vel_err; // Do the same calculation also when we are to close// maybe we should make this more smooth
    }

    if (vel_new < 0.0f) vel_new = 0.0f; 

	if (vel_new > _parameters.mpc_max_speed) 
        vel_new = _parameters.mpc_max_speed;

    dd_log.log(0,(double)_target_velocity);
    dd_log.log(1,(double)_drone_velocity);
    dd_log.log(2,(double)sp_velocity);
    dd_log.log(3,(double)dst_to_ok);
    dd_log.log(4,(double)max_vel_err);
    dd_log.log(5,(double)_trajectory_distance);
    dd_log.log(6,(double)_vel_ch_rate_f);
    dd_log.log(7,(double)vel_new);

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

bool PathFollow::check_current_trajectory_point_passed() {

    float acc_dst_to_line = _parameters.pafol_acc_dst_to_line;
    float acc_dst_to_point = _parameters.pafol_acc_dst_to_point;
    
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();
	global_pos = _navigator->get_global_position();

	struct map_projection_reference_s _ref_pos;
    map_projection_init(&_ref_pos, global_pos->lat, global_pos->lon);

    math::Vector<2> vel_xy(global_pos->vel_n, global_pos->vel_e);  
    math::Vector<2> dst_xy; 
    
    map_projection_project(&_ref_pos,
				pos_sp_triplet->current.lat, pos_sp_triplet->current.lon,
				&dst_xy.data[0], &dst_xy.data[1]);

    if (vel_xy.length() < 1e-6f) return false;

    float dot_product = vel_xy(0) * dst_xy(0) + vel_xy(1) * dst_xy(1);
    float dst_to_line = dot_product / vel_xy.length();

    if (dst_to_line <= acc_dst_to_line && dst_xy.length() <= acc_dst_to_point )
        return true;
    else
        return false;
}
