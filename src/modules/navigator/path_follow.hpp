/**
 * Path follow class
 */
#pragma once

// TODO! Consider migrating this class elsewhere
#include <containers/Queue.hpp>

#include "navigator_mode.h"

struct buffer_point_s {
	double lat, lon;
	float alt;
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
	// Be considerate - each point takes 24 bytes and we have about 42500 bytes free
	Queue<buffer_point_s> _saved_trajectory; // Saved trajectory points queue
	float _trajectory_distance; // Distance by trajectory points only, excluding drone or target current position
	buffer_point_s _actual_point; // Point currently in use as a setpoint
	bool _has_valid_setpoint; // Flag indicating if _actual_point can be used
	buffer_point_s _future_point; // Point of trajectory after the actual
	float _desired_speed; // Speed we want to move with until distance changes
	float _min_distance, _max_distance, _ok_distance; // Distances to use when following

	// Updates saved trajectory and trajectory distance with a new point
	void update_saved_trajectory();
	void update_setpoint(const buffer_point_s &desired_point, position_setpoint_s &destination);
	// Calculates desired speed in m/s based on current distance and target's speed
	float calculate_desired_speed(float distance);
	float calculate_current_distance();

};
