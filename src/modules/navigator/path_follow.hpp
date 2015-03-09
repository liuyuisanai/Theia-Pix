/**
 * Path follow class
 */
#pragma once

// TODO! Consider migrating this class elsewhere
#include <containers/Queue.hpp>
#include <mathlib/math/filter/LowPassFilter.hpp>
#include <sdlog2/debug_data_log.cpp>
#include <uORB/topics/vehicle_local_position.h>

#include "navigator_mode.h"

struct buffer_point_s {

	float x;
	float y;
	float z;

	// Provide memcopy assignment for the Queue class
	buffer_point_s& operator=(buffer_point_s const &copy) {
		memcpy(this, &copy, sizeof(buffer_point_s));
		return *this;
	}
};

// TODO! All modes use Mission Block as a superclass. Is it really needed?
class PathFollow : public NavigatorMode {
public:
	PathFollow(Navigator *navigator, const char *name);
	~PathFollow();
	bool init();
	// Inherited virtual methods
	virtual void on_inactive();
	virtual void on_activation();
	virtual void on_active();
	virtual void execute_vehicle_command();
private:
    int iter = 0;

    // Timestamp of the last trajectory point
	uint64_t _last_trajectory_time;             
    // Trajectory point Queue
	Queue<buffer_point_s> _traj_point_queue;    
    // Distance by trajectory points only, excluding drone or target current position
	float _trajectory_distance;                 
    // Latest point in the trajectory queue
	buffer_point_s _latest_point;               
    // Point currently in use as a setpoint
	buffer_point_s _active_traj_point;          
    // Point currently in use as a setpoint
	buffer_point_s tmp_point;                   
    // Last reached point 
    buffer_point_s _last_point;                 
    // Flag if drone is reaching for trajectory point.
	bool _reaching_for_traj_point;              
    // Point of trajectory after the actual
	buffer_point_s _future_point;               
    // Indicates if the mode was inited successfully
	bool _inited;                               

	int     _home_position_sub;
	int		_target_pos_sub;

    home_position_s _home_pos;

	struct map_projection_reference_s _ref_pos;
	float _ref_alt;
    float _starting_z;
    float _vertical_offset;

    debug_data_log dd_log;

	struct vehicle_global_position_s *_drone_global_pos;
    struct vehicle_local_position_s _drone_local_pos;

	struct target_global_position_s *_target_global_pos;
    struct vehicle_local_position_s _target_local_pos;

    float _drone_speed;
    float _target_speed;

    // Speed we want to move with until distance changes
	float _desired_speed;                   
    // Distances to use when following
	float _optimal_distance;                     
    // Distances to use when following
	float _break_distance;                     

    // desired speed PID controller integral component coefficient
    double _fp_i;                           
    // desired speed PID controller proportional component coefficient
    double _fp_p;                           
    // desired speed PID controller dirivetive component coefficient 
    double _fp_d;                           
    // desired speed PID controller second degree component coefficient 
    double _fp_dd;                          

    math::LowPassFilter<float> _fp_d_lpf;
    math::LowPassFilter<float> _vel_lpf;


    hrt_abstime _calc_vel_pid_t_prev;

    double _fp_p_last;

    hrt_abstime _last_dpos_t;
    hrt_abstime _last_tpos_t;

	// Updates saved trajectory and trajectory distance with a new point
	void update_traj_point_queue();
	// Update position setpoint to desired values
	inline void set_tpos_to_setpoint(position_setpoint_s &destination);
	// Update position setpoint to desired values
	inline void put_buffer_point_into_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination);
	// Update target velocity with a new value
	inline void update_target_velocity();
	// Update drone velocity with a new value
	inline void update_drone_velocity();
	// Calculates desired speed in m/s based on current distance and target's speed using PID controller
	inline float calculate_desired_velocity();
	// Calculates total current distance by trajectory + drone distance to setpoint + target distance to last point
	inline float calculate_current_distance();
	// Checks if the next point in the buffer is safe to use
	inline bool check_point_safe();
    // Setpoint reached specialized for path follow
    inline bool check_active_traj_point_reached();
    // Get drone global position and calculate local position if needed
    inline void update_drone_pos();
    // Get target global position and calculate local position if needed
    inline void update_target_pos();
    // Euclidean distance between two points
    inline float euclidean_distance(float x1, float y1, float x2, float y2);
    // Calculate altitude in local coordinates at which drone should be currently
    inline float calculate_desired_z();
};
