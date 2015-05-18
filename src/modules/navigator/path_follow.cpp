/**
 *
 * @file path_follow.cpp
 *
 * Path follow navigator mode implementation 
 *
 * @author Anthony Kenga <anton.k@airdog.vom>
 * @author Martins Frolovs <martins.f@airdog.vom>
 *
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

    _fp_i = 0.0f;
    _fp_p = 0.0f;
    _fp_d = 0.0f;

}

void PathFollow::on_activation() {

    _vstatus = _navigator->get_vstatus();

	_home_position_sub = orb_subscribe(ORB_ID(home_position));
    orb_copy(ORB_ID(home_position), _home_position_sub, &_home_pos);
    map_projection_init(&_ref_pos, _home_pos.lat, _home_pos.lon);

    _ref_alt = _home_pos.alt;

    update_drone_pos();
    update_target_pos();


	_mavlink_fd = _navigator->get_mavlink_fd();
    _optimal_distance = _parameters.pafol_optimal_dist;

    _last_passed_point.y = _drone_local_pos.y;
    _last_passed_point.x = _drone_local_pos.x;
    _last_passed_point.z = _target_local_pos.z;

    at_least_one_point = false;

    if (_parameters.follow_rpt_alt == 0) {
        _starting_z = _drone_local_pos.z;
        _vertical_offset = 0.0f;
    } 

    if (_parameters.follow_rpt_alt == 1) {
        _vertical_offset = _drone_local_pos.z - _target_local_pos.z;
    }


    _z_start = _drone_local_pos.z - _vertical_offset;
    _y_start = _drone_local_pos.y;
    _x_start = _drone_local_pos.x;

    
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

	_traj_point_queue.do_empty();
    _trajectory_distance = 0.0f;
    _first_tp_flag = false;
    _second_tp_flag = false;

	update_traj_point_queue();

	_navigator->invalidate_setpoint_triplet();

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

	pos_sp_triplet->next.valid = false;
	pos_sp_triplet->current.valid = true;
	pos_sp_triplet->previous.valid = false;

    put_tpos_into_setpoint(_target_global_pos, pos_sp_triplet->current);

	pos_sp_triplet->current.abs_velocity = 0.0f;
	pos_sp_triplet->current.abs_velocity_valid = true;
	_navigator->set_position_setpoint_triplet_updated();

}
void PathFollow::on_active() {

	if (!_inited) {
		return; // Wait for the Loiter mode to take over, but avoid pausing main navigator thread
	}

    _vstatus = _navigator->get_vstatus();

    update_drone_pos();
    update_target_pos();
	update_traj_point_queue();

	if ( update_vehicle_command() ) {
		execute_vehicle_command();
	}

	pos_sp_triplet = _navigator->get_position_setpoint_triplet();

    calculate_dst_to_gate();

    _tp_just_reached = false; 

    if (_first_tp_flag == true && traj_point_reached()) {

        _last_passed_point = _first_tp;
        _first_tp_flag = false;

        // Trajectory point has been reached. Current z is starting z for next trajectory segment.
        _z_start = _z_goal;
        _x_start = _drone_local_pos.x;
        _y_start = _drone_local_pos.y;

        _tp_just_reached = true; 

    }

    int it = 0;

    while (_first_tp_flag == false || _second_tp_flag == false){


        if (_first_tp_flag == false && _second_tp_flag == true) {

            _first_tp = _second_tp; 
            _second_tp_flag = false;
            _first_tp_flag = true;

            // _first_tp has changed so has dst_to_gate - recalculate it
            calculate_dst_to_gate();

            // calculate new z_start, z_goal
            calculate_alt_values(_tp_just_reached);

        }

        if (! (check_queue_ready() && _traj_point_queue.pop(_second_tp) ))
            break;

        _trajectory_distance -= _second_tp.distance;
        _second_tp_flag = true;

        if (++it>2) break;
    }


    if (euclidean_distance(_target_local_pos.x, _target_local_pos.y, _drone_local_pos.x, _drone_local_pos.y ) < _parameters.pafol_optimal_dist - 7.0f ){

        _traj_point_queue.do_empty();
        _trajectory_distance = 0.0f;

        _first_tp_flag = false;
        _second_tp_flag = false;

    }

    pos_sp_triplet->current.type = SETPOINT_TYPE_VELOCITY;

    if (_first_tp_flag){
        put_buffer_point_into_setpoint(_first_tp,pos_sp_triplet->current);
    } else {
        put_tpos_into_setpoint(_target_global_pos, pos_sp_triplet->current);
    }

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


        if (at_least_one_point)
            new_point.distance = euclidean_distance(_latest_added_tp.x, _latest_added_tp.y, new_point.x, new_point.y);

        if (!isfinite(new_point.distance))
            new_point.distance = 0.0f;
     
		if (_traj_point_queue.add(new_point, true)) {

            _latest_added_tp = new_point;
            _trajectory_distance += new_point.distance;
            at_least_one_point = true;

        } else {

			mavlink_log_critical(_mavlink_fd, "Trajectory overflow!");
			warnx("Trajectory overflow!");

		}
	}
}

void PathFollow::put_buffer_point_into_setpoint(const buffer_point_s &buffer_point, position_setpoint_s &setpoint) {

    map_projection_reproject(&_ref_pos, buffer_point.x, buffer_point.y, &setpoint.lat, &setpoint.lon);
	setpoint.position_valid = true;
	setpoint.valid = true;

}

void PathFollow::put_tpos_into_setpoint(target_global_position_s *&tpos, position_setpoint_s &setpoint_to) {

    setpoint_to.lat = tpos->lat;
    setpoint_to.lon = tpos->lon;
    setpoint_to.position_valid = true;
    setpoint_to.valid = true;

}

float PathFollow::calculate_desired_velocity() {

    float current_distance = calculate_current_distance();
    float dst_to_optimal = current_distance - _optimal_distance;

    float fp_i_coif = _parameters.pafol_vel_i;
    float fp_p_coif = _parameters.pafol_vel_p;
    float fp_d_coif = _parameters.pafol_vel_d;

    hrt_abstime t = hrt_absolute_time();
    float calc_vel_pid_dt = _calc_vel_pid_t_prev != 0 ? (t - _calc_vel_pid_t_prev) * 1e-6f : 0.0f;

    _fp_p_last = _fp_p;

    _fp_p = dst_to_optimal;
    _fp_i = _fp_i + _fp_p * calc_vel_pid_dt;

    if (calc_vel_pid_dt > 0.0f) {
        float fp_d_current = (_fp_p - _fp_p_last) / calc_vel_pid_dt;
        _fp_d = _fp_d_lpf.apply(t, fp_d_current);
    }

    // Solution to problem when accumulated error is very big, drone is to close and comes closer because of the accumulated error (integral part), which
    // is decreasing to slow in this case.

    // dst_to_optimal is below zero and trajectory distance is still decreasing[ because dirivative is smaller than zero], so the pulling power[integral part] is to big and needs 
    // some extra help to decrease it, we do it with factor pafol_vel_i_add_dec_rate [follow path velocity integral part aditional decrease rate]
    if (dst_to_optimal < -1.0f && _fp_d < -1.0f && _drone_speed > 2.0f) { 

        float additional_i_dec = _fp_d * _parameters.pafol_vel_i_add_dec_rate;
        if (additional_i_dec < 0.0f) additional_i_dec = -additional_i_dec;
        _fp_i -= additional_i_dec * calc_vel_pid_dt;

    }

    // In some cases the same extra help is needed to increase the integral part a bit faster.
    if (dst_to_optimal > 1.0f && _fp_d > 1.0f && _drone_speed > 2.0f) { 

        float additional_i_inc = _fp_d * _parameters.pafol_vel_i_add_inc_rate;
        if (additional_i_inc < 0.0f) additional_i_inc = -additional_i_inc;
        _fp_i += additional_i_inc * calc_vel_pid_dt;

    }

    if (_fp_i>_parameters.pafol_vel_i_upper_limit) _fp_i = _parameters.pafol_vel_i_upper_limit;
    if (_fp_i<_parameters.pafol_vel_i_lower_limit) _fp_i = _parameters.pafol_vel_i_lower_limit;

    float vel_new =_fp_i * fp_i_coif + _fp_p * fp_p_coif + _fp_d * fp_d_coif;

    // Let's prevent drone going backwards.
    if (!_drone_is_going_backwards && vel_new < 0.0f && _drone_speed_d > 0.0f) {
        _drone_is_going_backwards = true;

        going_bckw_st(0) = _drone_local_pos.x;
        going_bckw_st(1) = _drone_local_pos.y;

    }

    if (_drone_is_going_backwards && vel_new > 1.0f){
        _drone_is_going_backwards = false;
    }

    if (_drone_is_going_backwards) {

        if (vel_new < -1.0f)
            vel_new = -1.0f;

        if (!(going_bckw_st(0) == _drone_local_pos.x && going_bckw_st(1) == _drone_local_pos.y) && 
                euclidean_distance(going_bckw_st(0), going_bckw_st(1), _drone_local_pos.x, _drone_local_pos.y)> _parameters.pafol_backward_distance_limit )
        {
            vel_new = 0.0f;

            if (_fp_i < -10.0f)       
                _fp_i = -10.0f;
        }

    }

    dd_log.log(0,_fp_i);
    dd_log.log(1,_fp_p);
    dd_log.log(2,_fp_d);

    // mavlink_log_info(_mavlink_fd, "i:%.3f, p:%.3f d:%.3f", (double)fp_d_coif, (double)fp_p_coif, (double)fp_i_coif);
    // mavlink_log_info(_mavlink_fd, "dst:%.3f, fp_i:%.3f fp_d:%.3f, ad:%.3f", (double)dst_to_optimal, (double)_fp_i, (double)_fp_d);
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

    float tpos_dt = _last_tpos_t != 0 ? (t - _last_tpos_t) * 1e-6f : 0.0f;
    float dpos_dt = _last_dpos_t != 0 ? (t - _last_dpos_t) * 1e-6f : 0.0f;

    if (tpos_dt > 2.0f) tpos_dt = 2.0f;
    if (dpos_dt > 2.0f) dpos_dt = 2.0f;

    target_lpos = target_lpos + target_vel * tpos_dt;
    drone_lpos = drone_lpos + drone_vel * dpos_dt;

    float rate_a = 0.0f;
    float rate_b = 0.0f;

    if (_first_tp_flag == true) {

            float acc_dst_to_gate = _parameters.pafol_acc_dst_to_gate;
            float traj_radius = _parameters.airdog_traj_radius;

            if (_dst_to_gate > traj_radius) 
                rate_a = 1.0f;
            else if (_dst_to_gate < acc_dst_to_gate) 
                rate_a = 0.0f;
            else 
                rate_a = (_dst_to_gate - acc_dst_to_gate) / (traj_radius - acc_dst_to_gate);
            
            rate_b = 1.0f - rate_a;
    }

        if (_first_tp_flag == false){
        // No trajectory points

            full_distance = euclidean_distance(drone_lpos(0), drone_lpos(1), target_lpos(0), target_lpos(1));

        } else if (_first_tp_flag == true && _second_tp_flag == false) {
        // One trajectory point

            float distance_a = euclidean_distance(drone_lpos(0), drone_lpos(1), _first_tp.x, _first_tp.y) + 
                            euclidean_distance(_first_tp.x, _first_tp.y, target_lpos(0), target_lpos(1));

            float distance_b = euclidean_distance(drone_lpos(0), drone_lpos(1), target_lpos(0), target_lpos(1));

            full_distance = rate_a * distance_a  + rate_b * distance_b;

         
        } else {
        // Two or more trajectory points

            float distance_a = euclidean_distance(drone_lpos(0), drone_lpos(1), _first_tp.x, _first_tp.y) + 
                            euclidean_distance(_first_tp.x, _first_tp.y, _second_tp.x, _second_tp.y);

            float distance_b = euclidean_distance(drone_lpos(0), drone_lpos(1), _second_tp.x, _second_tp.y);

            full_distance = rate_a * distance_a  + rate_b * distance_b + 
                            _trajectory_distance + 
                            euclidean_distance(_latest_added_tp.x, _latest_added_tp.y, target_lpos(0), target_lpos(1));

            // dd_log.log(5,distance_a);
            // dd_log.log(6,distance_b);
            // dd_log.log(7,rate_a);

        }

    return full_distance;
}

bool PathFollow::check_queue_ready() {

	buffer_point_s proposed_point;
	if (!_traj_point_queue.peek(0, proposed_point)) {
		return false;
	}
    return true;
}

bool PathFollow::traj_point_reached() {

    float acc_dst_to_gate = _parameters.pafol_acc_dst_to_gate;
    float gate_width = _parameters.pafol_gate_width;
   
    calculate_dst_to_gate();

    if (_dst_to_gate <= acc_dst_to_gate && abs(_dst_to_tunnel_middle) * 2.0f <= gate_width )
        return true;
    else
        return false;

}

void PathFollow::calculate_dst_to_gate(){

    // Vector from last trajectory_point to next one.
    math::Vector<2> tunnel(_first_tp.x - _last_passed_point.x, _first_tp.y - _last_passed_point.y);
    // Vector from current drone position to next trajectory_point.
    math::Vector<2> drone_to_tp(_first_tp.x - _drone_local_pos.x, _first_tp.y - _drone_local_pos.y);

    // dot_p(A,B) = |A|x|B| x cos a
    float dot_product = tunnel(0) * drone_to_tp(0) + tunnel(1) * drone_to_tp(1);
    // cross_p(A,B) = |A|x|B| x sin a
    float cross_product = tunnel(0) * drone_to_tp(1) - tunnel(1) * drone_to_tp(0);

    _dst_to_gate = 0.0f;
    _dst_to_tunnel_middle = 0.0f;

    if (abs(tunnel(0)) > 1e-6f || abs(tunnel(1)) > 1e-6f) {
        // Resultig in |B| x cos a
        _dst_to_gate = dot_product /tunnel.length();
        // Resultig in |B| x sin a
        _dst_to_tunnel_middle = cross_product /tunnel.length();
    }

}

 void
PathFollow::update_target_pos(){

    
    if (_vstatus->condition_target_position_valid)
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

        _last_tpos_t = hrt_absolute_time();

        _target_speed = sqrt ( _target_local_pos.vx * _target_local_pos.vx + _target_local_pos.vy * _target_local_pos.vy);
    }
}

void
PathFollow::update_drone_pos(){

    if (_vstatus->condition_global_position_valid)
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

        _last_dpos_t = hrt_absolute_time();


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

    float ret_z = 0.0f;

    if (_parameters.follow_rpt_alt == 0) {

        ret_z = _starting_z;

    } else { 

        float length_full = 0.0f;
        float length_left = 0.0f;
        float rate_done = 0.0f;
        float rate_left = 0.0f;

        // Go for trajectory point
        if (_first_tp_flag){

            length_full = _z_start_dst_to_gate - _parameters.pafol_acc_dst_to_gate;
            length_left = _dst_to_gate - _parameters.pafol_acc_dst_to_gate;
            rate_left = length_left / length_full;
            rate_done = 1.0f - rate_left;

            if (rate_done < 0.0f) rate_done = 0.0f;
            ret_z = _z_start + ( _z_goal - _z_start) * rate_done;

            dd_log.log(3,_dst_to_gate);

        // Follow target - no trajectory points
        } else {

            length_full = euclidean_distance(_x_start, _y_start, _target_local_pos.x, _target_local_pos.y);
            length_left = euclidean_distance(_drone_local_pos.x, _drone_local_pos.y, _target_local_pos.x, _target_local_pos.y);
            rate_left = length_left / length_full;
            rate_done = 1.0f - rate_left;

            if (rate_done < 0.0f) rate_done = 0.0f;
            ret_z = _z_start + (_target_local_pos.z - _z_start) * rate_done;

            dd_log.log(3,2);
        } 

        dd_log.log(4,length_full);

        dd_log.log(5,length_left);
        dd_log.log(6,rate_done);
        dd_log.log(7,ret_z);

        
        //mavlink_log_info(_mavlink_fd, "zs:%.3f, zg:%.3f rz:%.3f, ll:%.3f", (double)_z_start, (double)_z_goal, (double)ret_z, (double)length_left);

    }

    return ret_z + _vertical_offset;
}

void
PathFollow::calculate_alt_values(bool tp_just_reached){

    if (!tp_just_reached)
        _z_start = _drone_local_pos.z - _vertical_offset;

    _x_start = _drone_local_pos.x;
    _y_start = _drone_local_pos.y;

    _z_start_dst_to_gate = _dst_to_gate;

    if (_z_start < _first_tp.z){

        float dst_to_reaching_point = _dst_to_gate - _parameters.pafol_acc_dst_to_gate;
        float rate = dst_to_reaching_point / _dst_to_gate;

        _z_goal = _z_start + (_first_tp.z - _z_start) * rate;
    
    } else {
        _z_goal = _first_tp.z;
    }

}

