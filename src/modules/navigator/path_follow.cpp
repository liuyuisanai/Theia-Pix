/**
 *
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
		_traj_point_queue(),
		_trajectory_distance(0.0f),
		_reaching_for_traj_point(false),
		_inited(false),
        _drone_speed_d(0.0f),
        _drone_is_going_backwards(false),
		_desired_speed(0.0f),
		_optimal_distance(0.0f),
        _fp_i(0.0f),
        _fp_p(0.0f),
        _fp_d(0.0f),
        _fp_d_lpf(0.2),
        _vel_lpf(0.1),
        _fp_p_last(0.0f),
        _last_dpos_t(0.0f),
        _last_tpos_t(0.0f)
        {
}
PathFollow::~PathFollow() {

}
bool PathFollow::init() {
	updateParameters();

	_inited = _traj_point_queue.init(_parameters.pafol_buf_size);

	return (_inited);
}
void PathFollow::on_inactive() {
}

void PathFollow::on_activation() {

	_home_position_sub = orb_subscribe(ORB_ID(home_position));
    orb_copy(ORB_ID(home_position), _home_position_sub, &_home_pos);
    map_projection_init(&_ref_pos, _home_pos.lat, _home_pos.lon);

    _ref_alt = _home_pos.alt;

    update_drone_pos();
    update_target_pos();

	_mavlink_fd = _navigator->get_mavlink_fd();
    _optimal_distance = _parameters.pafol_optimal_dist;

    _last_point.y = _drone_local_pos.y;
    _last_point.x = _drone_local_pos.x;
    _last_point.z = _target_local_pos.z;

    if (_parameters.follow_rpt_alt == 0) {
        _starting_z = _drone_local_pos.z;
        _vertical_offset = 0.0f;
    } 
    
    if (_parameters.follow_rpt_alt == 1) {
        _vertical_offset = _drone_local_pos.z - _target_local_pos.z;
    }

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

	_reaching_for_traj_point = false;
    _trajectory_distance = 0.0f;

	_traj_point_queue.do_empty();
	update_traj_point_queue();

	_navigator->invalidate_setpoint_triplet();
	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	pos_sp_triplet->next.valid = false;
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->previous.valid = false;

    set_tpos_to_setpoint(pos_sp_triplet->current);

	pos_sp_triplet->current.abs_velocity = 0.0f;
	pos_sp_triplet->current.abs_velocity_valid = true;
	_navigator->set_position_setpoint_triplet_updated();

}
void PathFollow::on_active() {

	if (!_inited) {
		return; // Wait for the Loiter mode to take over, but avoid pausing main navigator thread
	}

    update_drone_pos();
    update_target_pos();
	update_traj_point_queue();

	if ( update_vehicle_command() ) {
		execute_vehicle_command();
	}

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

    if (_reaching_for_traj_point) {
        if (check_active_traj_point_reached()) {
            _last_point = _active_traj_point;
            if (_traj_point_queue.pop(tmp_point)){
                _reaching_for_traj_point = true;
                _trajectory_distance -= euclidean_distance(_active_traj_point.x, _active_traj_point.y, tmp_point.x, tmp_point.y);
                _active_traj_point = tmp_point;
                put_buffer_point_into_setpoint(_active_traj_point, pos_sp_triplet->current);
            } else {
                _reaching_for_traj_point = false;
                set_tpos_to_setpoint(pos_sp_triplet->current);
            }
        } 
    } else {
        set_tpos_to_setpoint(pos_sp_triplet->current);
        if (check_point_safe()) {
            if (_traj_point_queue.pop(tmp_point)){
                _reaching_for_traj_point = true;
                _active_traj_point = tmp_point;
                put_buffer_point_into_setpoint(_active_traj_point, pos_sp_triplet->current);
            }
        }
    }

    if (euclidean_distance(_target_local_pos.x, _target_local_pos.y, _drone_local_pos.x, _drone_local_pos.y ) < _parameters.pafol_optimal_dist - 7.0f )
        _traj_point_queue.do_empty();

    pos_sp_triplet->current.abs_velocity = calculate_desired_velocity();
    pos_sp_triplet->current.alt = _ref_alt - calculate_desired_z();

    _navigator->set_position_setpoint_triplet_updated();
}

void PathFollow::execute_vehicle_command() {
	if (_vcommand.command == VEHICLE_CMD_DO_SET_MODE){

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
				_optimal_distance += _parameters.horizon_button_step;
				break;
			case REMOTE_CMD_CLOSER: {
				_optimal_distance -= _parameters.horizon_button_step;
				break;
			}
			case REMOTE_CMD_UP: {
				_vertical_offset -= _parameters.up_button_step;
				break;
			}
			case REMOTE_CMD_DOWN: {
				_vertical_offset += _parameters.down_button_step;
                break;
			}
		}
	}
}

void PathFollow::update_traj_point_queue() {

	struct external_trajectory_s *target_trajectory = _navigator->get_target_trajectory();

	if (_last_trajectory_time != target_trajectory->timestamp && target_trajectory->point_type != 0) {

        buffer_point_s new_point;

		_last_trajectory_time = target_trajectory->timestamp;

		map_projection_project(&_ref_pos,
				target_trajectory->lat, target_trajectory->lon,
				&new_point.x, &new_point.y);

        new_point.z = _ref_alt - target_trajectory->alt;
     
		if (_traj_point_queue.add(new_point, true)) {

            if (_traj_point_queue.get_value_count() > 1 || _reaching_for_traj_point) {
                _trajectory_distance += euclidean_distance(_latest_point.x, _latest_point.y, new_point.x, new_point.y); 
            } else {
                _trajectory_distance = 0.0f;
            }             
            
            _latest_point = new_point;

        } else {
			mavlink_log_critical(_mavlink_fd, "Trajectory overflow!");
			warnx("Trajectory overflow!");
		}

	}
}

void PathFollow::put_buffer_point_into_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination) {

	destination.type = SETPOINT_TYPE_VELOCITY;
    map_projection_reproject(&_ref_pos, desired_point.x, desired_point.y, &destination.lat, &destination.lon);

	destination.position_valid = true;
	destination.valid = true;
}

void PathFollow::set_tpos_to_setpoint(position_setpoint_s &destination) {

    destination.type = SETPOINT_TYPE_VELOCITY;
    destination.lat = _target_global_pos->lat;
    destination.lon = _target_global_pos->lon;

    destination.position_valid = true;
    destination.valid = true;

}

float PathFollow::calculate_desired_velocity() {

    float current_distance = calculate_current_distance();
    double dst_to_optimal = current_distance - _optimal_distance;

    double fp_i_coif = _parameters.pafol_vel_i;
    double fp_p_coif = _parameters.pafol_vel_p;
    double fp_d_coif = _parameters.pafol_vel_d;

    hrt_abstime t = hrt_absolute_time();
    double calc_vel_pid_dt = _calc_vel_pid_t_prev != 0 ? (t - _calc_vel_pid_t_prev) : 0.0f;

    calc_vel_pid_dt /= 1000000.0;

    _fp_p_last = _fp_p;

    _fp_p = dst_to_optimal;
    _fp_i = _fp_i + _fp_p * calc_vel_pid_dt;

    if (calc_vel_pid_dt > 0.0) {
        double fp_d_current = (_fp_p - _fp_p_last) / calc_vel_pid_dt;
        _fp_d = _fp_d_lpf.apply(t, (float)fp_d_current);
    }

    // Solution to problem when accumulated error is very big and drone is already to closer and comes closer because of the accumulated error (integral part), which
    // is decreasing to slow in this case.
    //
    // The same for drone going far and with very small negative accumulated error

    // dst_to_optimal is below zero and trajectory distance is still decreasing[ because dirivative is smaller than zero], so the pulling power is to big. 
    // integral part [accumulated error, the pulling power] is to big and it needs to be decreased faster
    if (dst_to_optimal < 0.0 && _fp_d < 0.0) { 

        double additional_i_dec = _fp_d * (double)_parameters.pafol_vel_i_add_dec_rate;
        if (additional_i_dec < 0.0) additional_i_dec = -additional_i_dec;
        _fp_i -= additional_i_dec * calc_vel_pid_dt;

    }

    // dst_to_optimal is above zero and distance is still increasing[ dirivative bigger than zero], 
    // integral part [accumulated error, the pulling power] is to small and it needs to be increased faster
    if (dst_to_optimal > 0.0 && _fp_d > 0.0) { 

        double additional_i_inc = _fp_d * (double)_parameters.pafol_vel_i_add_inc_rate;
        if (additional_i_inc < 0.0) additional_i_inc = -additional_i_inc;
        _fp_i += additional_i_inc * calc_vel_pid_dt;

    }

    if (_fp_i>(double)_parameters.pafol_vel_i_upper_limit) _fp_i = _parameters.pafol_vel_i_upper_limit;
    if (_fp_i<(double)_parameters.pafol_vel_i_lower_limit) _fp_i = _parameters.pafol_vel_i_lower_limit;

    double vel_new =_fp_i * fp_i_coif + _fp_p * fp_p_coif + _fp_d * fp_d_coif;

    // Let's prevent drone going backwards.
    if (!_drone_is_going_backwards && vel_new < 0.0 && _drone_speed_d > 0.0f) {
        _drone_is_going_backwards = true;
        //mavlink_log_info(_mavlink_fd, "d:%.3f",(double)_drone_is_going_backwards);
    }

    if (_drone_is_going_backwards && vel_new > 0.0){
        _drone_is_going_backwards = false;
    };

    if (_drone_is_going_backwards) {
        if (vel_new < -1.0)
            vel_new = -1.0;
    }

    dd_log.log(0,_fp_i);
    dd_log.log(1,_fp_p);
    dd_log.log(2,_fp_d);
    dd_log.log(3,vel_new);
    dd_log.log(4,_drone_speed);
    dd_log.log(5,_target_speed);
    dd_log.log(6,dst_to_optimal);

    // mavlink_log_info(_mavlink_fd, "i:%.3f, p:%.3f d:%.3f", (double)fp_d_coif, (double)fp_p_coif, (double)fp_i_coif);
    //mavlink_log_info(_mavlink_fd, "dst:%.3f, fp_i:%.3f fp_d:%.3f, ad:%.3f", (double)dst_to_optimal, (double)_fp_i, (double)_fp_d);
    // mavlink_log_info(_mavlink_fd, "ul:%.3f, ll:%.3f ir:%.3f dr:%.3f", (double)_parameters.pafol_vel_i_lower_limit, (double)_parameters.pafol_vel_i_upper_limit, 
    //        (double)_parameters.pafol_vel_i_add_inc_rate, (double)_parameters.pafol_vel_i_add_dec_rate);
    _calc_vel_pid_t_prev = t;

    return vel_new;
}


float PathFollow::calculate_current_distance() {

    float full_distance = 0.0f;

    math::Vector<2> target_lpos(_target_local_pos.x, _target_local_pos.y);
    math::Vector<2> target_vel(_target_local_pos.vx,_target_local_pos.vy);

    math::Vector<2> drone_lpos(_drone_local_pos.x, _drone_local_pos.y);
    math::Vector<2> drone_vel(_drone_local_pos.vx,_drone_local_pos.vy);

    hrt_abstime t = hrt_absolute_time();

    float tpos_dt = _last_tpos_t != 0 ? (t - _last_tpos_t) : 0.0f;
    float dpos_dt = _last_dpos_t != 0 ? (t - _last_dpos_t) : 0.0f;

    tpos_dt/= 1000000.0f;
    dpos_dt/= 1000000.0f;

    if (tpos_dt > 0.2f) tpos_dt = 0.2f;
    if (dpos_dt > 0.2f) dpos_dt = 0.2f;

    target_lpos = target_lpos + target_vel * tpos_dt;
    drone_lpos = drone_lpos + drone_vel * dpos_dt;

    if (_reaching_for_traj_point){

        full_distance = _trajectory_distance + 
            euclidean_distance(drone_lpos(0), drone_lpos(1), _active_traj_point.x, _active_traj_point.y) +
            euclidean_distance(_latest_point.x, _latest_point.y, target_lpos(0), target_lpos(1));

        //float tcount = _traj_point_queue.get_value_count();
        //mavlink_log_info(_mavlink_fd, "d:%.3f,td:%.3f, tx:%.3f,ty:%.3f, pcount:%.3f",(double)full_distance, (double)_trajectory_distance,(double)target_lpos(0), (double)target_lpos(1), (double)tcount);

        //mavlink_log_info(_mavlink_fd, "d:%.3f, ax:%.3f ay:%.3f,dx:%.3f,dy:%.3f,tx:%.3f,ty:%.3f",(double)full_distance, (double)_active_traj_point.x, (double)_active_traj_point.y, (double)drone_lpos(0), (double)drone_lpos(1), (double)target_lpos(0), (double)target_lpos(1));

    } else {

        full_distance = euclidean_distance(target_lpos(0), target_lpos(1), drone_lpos(0), drone_lpos(1));

        //mavlink_log_info(_mavlink_fd, "d:%.5f,dx:%.3f,dy:%.3f,tx:%.3f,ty:%.3f",(double)full_distance, (double)drone_lpos(0), (double)drone_lpos(1), (double)target_lpos(0), (double)target_lpos(1));


    }

    return full_distance;

}

bool PathFollow::check_point_safe() {

	buffer_point_s proposed_point;
	if (!_traj_point_queue.peek(0, proposed_point)) {
		return false;
	}
    return true;

}

bool PathFollow::check_active_traj_point_reached() {

    float acc_dst_to_line = _parameters.pafol_acc_dst_to_line;
    float acc_dst_to_point = _parameters.pafol_acc_dst_to_point;

    math::Vector<2> vel_xy(_drone_local_pos.vx, _drone_local_pos.vy);  
    math::Vector<2> dst_xy(_active_traj_point.x - _drone_local_pos.x, _active_traj_point.y - _drone_local_pos.y); 

    if (vel_xy.length() < 1e-6f) return false;

    float dot_product = vel_xy(0) * dst_xy(0) + vel_xy(1) * dst_xy(1);
    float dst_to_line = dot_product / vel_xy.length();

    if (dst_to_line <= acc_dst_to_line && dst_xy.length() <= acc_dst_to_point )
        return true;
    else
        return false;
}

void
PathFollow::update_target_pos(){

    _target_global_pos = _navigator->get_target_position();

    if (_target_global_pos->timestamp != _target_local_pos.timestamp){

        _target_local_pos.timestamp = _target_global_pos->timestamp;

        _target_local_pos.vx = _target_global_pos->vel_n;
        _target_local_pos.vy = _target_global_pos->vel_e;
        _target_local_pos.vz = _target_global_pos->vel_d;

        _target_local_pos.eph = _target_global_pos->eph;
        _target_local_pos.epv = _target_global_pos->epv;

		map_projection_project(&_ref_pos,
				_target_global_pos->lat, _target_global_pos->lon,
				&_target_local_pos.x, &_target_local_pos.y);

        _target_local_pos.z = _ref_alt - _target_global_pos->alt;

        _last_tpos_t = hrt_abstime();

        _target_speed = sqrt ( _target_local_pos.vx * _target_local_pos.vx + _target_local_pos.vy * _target_local_pos.vy);
    }
}

void
PathFollow::update_drone_pos(){

	_drone_global_pos = _navigator->get_global_position();

    if (_drone_global_pos->timestamp != _drone_local_pos.timestamp){


        uint64_t last_timestamp  = _drone_local_pos.timestamp;		
        float last_drone_speed = _drone_speed;

        _drone_local_pos.timestamp = _drone_global_pos->timestamp;

        _drone_local_pos.vx = _drone_global_pos->vel_n;
        _drone_local_pos.vy = _drone_global_pos->vel_e;
        _drone_local_pos.vz = _drone_global_pos->vel_d;

        _drone_local_pos.eph = _drone_global_pos->eph;
        _drone_local_pos.epv = _drone_global_pos->epv;

		map_projection_project(&_ref_pos,
				_drone_global_pos->lat, _drone_global_pos->lon,
				&_drone_local_pos.x, &_drone_local_pos.y);

        _drone_local_pos.z = _ref_alt - _drone_global_pos->alt;

        _last_dpos_t = hrt_abstime();


        _drone_speed = sqrt ( _drone_local_pos.vx * _drone_local_pos.vx + _drone_local_pos.vy * _drone_local_pos.vy);

		float dt = last_timestamp != 0 ? (_drone_local_pos.timestamp - last_timestamp) * 1e-6f : 0.0f;

        if (dt > 1e-7f)
            _drone_speed_d = (_drone_speed - last_drone_speed) / dt;
        else 
            _drone_speed_d = 0.0f;

    }
}

float
PathFollow::euclidean_distance(float x1,float y1, float x2, float y2){

    float dx = x2 - x1;
    float dy = y2 - y1;

    return sqrt( dx*dx + dy*dy );

}

float
PathFollow::calculate_desired_z() {

    float ret_z;

    if (_parameters.follow_rpt_alt == 0) {

        ret_z = _starting_z;

    } else { 

        float xy_full = 0.0f;
        float xy_part = 0.0f;
        float z_full = 0.0f;
        float z_part = 0.0f;

        if (_reaching_for_traj_point){

            z_full = _active_traj_point.z - _last_point.z;

            xy_full = euclidean_distance(_last_point.x, _last_point.y, _active_traj_point.x, _active_traj_point.y);
            xy_part = euclidean_distance(_drone_local_pos.x, _drone_local_pos.y, _active_traj_point.x, _active_traj_point.y);

            if (xy_full < 0.1f) 
                z_part = 0.0f;
            else
                z_part = ( z_full * xy_part ) / xy_full;

            ret_z = _active_traj_point.z - z_part;

        } else {

            z_full = _target_local_pos.z - _last_point.z;

            xy_full = euclidean_distance(_last_point.x, _last_point.y, _target_local_pos.x, _target_local_pos.y);
            xy_part = euclidean_distance(_drone_local_pos.x, _drone_local_pos.y, _target_local_pos.x, _target_local_pos.y);


            if (xy_full < 0.1f) 
                z_part = 0.0f;
            else
                z_part = ( z_full * xy_part ) / xy_full;

            
            ret_z = _target_local_pos.z - z_part;
        } 
    }

    return ret_z + _vertical_offset;
}

