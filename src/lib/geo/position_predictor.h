/*
 * position_predictor.h
 *
 *  Created on: 04 июля 2014 г.
 *      Author: ton
 */

#pragma once

#include <stdint.h>
#include "visibility.h"


class BasePredictor {
public:
	/**
	 * Constructor
	 */
	BasePredictor();

	/**
	 * Get latency averaging number, samples
	 */
	float get_avg_n() const { return _avg_n; }

	/**
	 * Set latency averaging number, samples
	 */
	void set_avg_n(const float avg_n) { _avg_n = avg_n; }

	/**
	 * Get minimal, known a priory latency, us
	 */
	uint64_t get_min_latency() const { return _latency_min; }

	/**
	 * Set minimal known a priori latency, us
	 */
	void set_min_latency(const uint64_t latency) { _latency_min = latency; }

	/**
	 * Get maximal allowed latency, us
	 */
	uint64_t get_max_latency() const { return _latency_max; }

	/**
	 * Set maximal allowed latency, us
	 */
	void set_max_latency(const uint64_t latency) { _latency_max = latency; }

	/**
	 * Get time when last update was received, us
	 */
	uint64_t get_time_recv_last() const { return _time_recv_last; };

	/**
	 * Get estimated time when last update was sent (in local clock reference), us
	 */
	uint64_t get_time_sent_last() const { return _time_sent_last; };

protected:
	/**
	 * Update last known position
	 */
	void update_latency(uint64_t time_local, uint64_t time_remote);

	uint64_t	_time_recv_last;			///< last time when last update was received, us
	uint64_t	_time_sent_last;			///< estimated time when last update was sent (in local clock reference), us

private:
	int64_t		_time_offset;		///< average time offset (time_local - time_remote), us
	uint64_t	_latency_min;		///< minimal, known a priori latency, us
	uint64_t	_latency_max;		///< maximal allowed latency, us
	float		_avg_n;				///< averaging number, samples
};


class __EXPORT GlobalPositionPredictor : public BasePredictor {
public:
	/**
	 * Constructor
	 */
	GlobalPositionPredictor();

	/**
	 * Update last known position
	 */
	void update(uint64_t time_local, uint64_t time_remote,
			double lat, double lon, float alt,
			const float vel[3]);

	/**
	 * Predict position at specified time
	 *
	 * @return time delta since last known data, us
	 */
	uint64_t predict_position(uint64_t time_local, double *lat, double *lon, float *alt) const;

private:
	double	_lat;				///< last known latitude, deg WSG84
	double	_lon;				///< last known longitude, deg WSG84
	float	_alt;				///< last known altitude, m AMSL
	float	_vel[3];			///< last known velocity, m/s NED
};


class __EXPORT LocalPositionPredictor : public BasePredictor {
public:
	/**
	 * Constructor
	 */
	LocalPositionPredictor();

	/**
	 * Update last known position
	 */
	void update(uint64_t time_local, uint64_t time_remote,
			const float pos[3], const float vel[3]);

	/**
	 * Predict position at specified time
	 *
	 * @return time delta since last known data, us
	 */
	uint64_t predict_position(uint64_t time_local, float pos[3]) const;

private:
	float	_pos[3];			///< last known position, m AMSL
	float	_vel[3];			///< last known velocity, m/s NED
};
