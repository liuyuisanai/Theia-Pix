/**
 * Path follow class
 */
#pragma once

// TODO! Consider migrating this class elsewhere
#include <containers/Queue.hpp>
#include <mathlib/math/filter/LowPassFilter.hpp>
#include <sdlog2/debug_data_log.cpp>

#include "navigator_mode.h"

struct buffer_point_s {
	double lat, lon;
	float alt;
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
	uint64_t _last_trajectory_time; // Timestamp of the last trajectory point
	buffer_point_s _last_point; // Last saved point
	Queue<buffer_point_s> _saved_trajectory; // Saved trajectory points queue
	float _trajectory_distance; // Distance by trajectory points only, excluding drone or target current position
	buffer_point_s _actual_point; // Point currently in use as a setpoint
	bool _has_valid_setpoint; // Flag indicating if _actual_point can be used
	buffer_point_s _future_point; // Point of trajectory after the actual
	float _vertical_offset; // Vertical offset off the trajectory
	bool _inited; // Indicates if the mode was inited successfully
	math::LowPassFilter<math::Vector<3>> _target_vel_lpf; // Filter for target velocity
	math::LowPassFilter<math::Vector<3>> _drone_vel_lpf; // Filter for target velocity
    math::LowPassFilter<float> _vel_ch_rate_lpf; // drone velocity change rate[acceleration] lpf
    math::Vector<3> _target_velocity_vect; // LPFed target velocity

    debug_data_log dd_log;

    struct position_setpoint_triplet_s 	*last_moving_sp_triplet;


    float _target_velocity;   // raw target velocity
    float _target_velocity_f; // target velocity filtered
    float _drone_velocity;    // raw drone velocity
    float _drone_velocity_f;    // drone velocity filtered 
    float _last_drone_velocity_f; // drone velocity in the last iteratiovn
    float _last_drone_velocity; // drone velocity in the last iteratiovn

	float _desired_speed; // Speed we want to move with until distance changes
	float _ok_distance; // Distances to use when following

    float _vel_ch_rate;  // velocity change rate
    float _vel_ch_rate_f; // velocity change rate filtered

    uint64_t _global_pos_timestamp_last;

    int itr=0;

	hrt_abstime _calc_vel_t_prev = 0;

	hrt_abstime _t_prev = 0;
	hrt_abstime _dt = 0;

    bool current_point_passed = false;
    bool zero_setpoint = false;

    float _alt;
    int co = 0;


	// Updates saved trajectory and trajectory distance with a new point
	void update_saved_trajectory();
	// Update position setpoint to desired values
	inline void set_target_setpoint(position_setpoint_s &destination);
	// Update position setpoint to desired values
	inline void put_buffer_point_into_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination);
	// Update target velocity with a new value
	inline void update_target_velocity();
	// Update drone velocity with a new value
	inline void update_drone_velocity();
	// Calculates desired speed in m/s based on current distance and target's speed
	inline float calculate_desired_velocity(float distance);
	// Calculates total current distance by trajectory + drone distance to setpoint + target distance to last point
	inline float calculate_current_distance();
	// Checks if the next point in the buffer is safe to use
	inline bool check_point_safe();
    // Setpoint reached specialized for path follow
    inline bool check_current_trajectory_point_passed();
};
