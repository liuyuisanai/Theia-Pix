/**
 * Path follow class
 */
#pragma once

// TODO! Consider migrating this class elsewhere
#include <containers/Queue.hpp>
#include <mathlib/math/filter/LowPassFilter.hpp>

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
	uint64_t _last_trajectory_time; // Timestamp of the last trajectory point
	buffer_point_s _last_point; // Last saved point
	Queue<buffer_point_s> _saved_trajectory; // Saved trajectory points queue
	float _trajectory_distance; // Distance by trajectory points only, excluding drone or target current position
	buffer_point_s _actual_point; // Point currently in use as a setpoint
	bool _has_valid_setpoint; // Flag indicating if _actual_point can be used
	buffer_point_s _future_point; // Point of trajectory after the actual
	float _desired_speed; // Speed we want to move with until distance changes
	float _min_distance, _max_distance, _ok_distance; // Distances to use when following
	float _vertical_offset; // Vertical offset off the trajectory
	bool _inited; // Indicates if the mode was inited successfully
	math::LowPassFilter<float> _target_vel_lpf; // Filter for target velocity
	float _target_velocity; // LPFed target velocity


	// Updates saved trajectory and trajectory distance with a new point
	void update_saved_trajectory();
	// Update position setpoint to desired values
	inline void update_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination);
	// Update target velocity with a new value
	inline void update_target_velocity();
	// Updates minimum and maximum distance based on ok distance
	inline void update_min_max_dist();
	// Calculates desired speed in m/s based on current distance and target's speed
	inline float calculate_desired_speed(float distance);
	// Calculates total current distance by trajectory + drone distance to setpoint + target distance to last point
	inline float calculate_current_distance();
	// Checks if the next point in the buffer is safe to use
	inline bool check_point_safe();

};
