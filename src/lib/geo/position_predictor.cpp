/*
 * position_predictor.cpp
 *
 *  Created on: 04 июля 2014 г.
 *      Author: ton
 */

#include <math.h>
#include <string.h>

#include "position_predictor.h"
#include "geo.h"

GlobalPositionPredictor::GlobalPositionPredictor() :
	_time_last(0),
	_time_offset(0),
	_latency_min(0),
	_latency_max(1000000),
	_avg_n(100),
	_lat(0.0),
	_lon(0.0),
	_alt(0.0f)
{}

void
GlobalPositionPredictor::update(uint64_t time_local, uint64_t time_remote,
		double lat, double lon, float alt,
		const float vel[3])
{
	int64_t offset = time_local - time_remote;
	int64_t latency = offset - _time_offset;

	if (_time_last == 0 || latency < _latency_min || latency > _latency_max) {
		/* time offset initialization
		 * or latency is less than possible
		 * or latency if bigger than allowed,
		 * correct time offset immediately
		 */
		_time_offset = offset - _latency_min;

	} else {
		/* time offset correction */
		if (latency < _latency_min) {
			_time_offset = offset - _latency_min;

		} else {
			/* latency is bigger than possible, correct time offset slowly */
			_time_offset += (offset - _latency_min - _time_offset) / _avg_n;
		}
	}

	_time_last = time_remote + _time_offset;
	_lat = lat;
	_lon = lon;
	_alt = alt;
	memcpy(_vel, vel, sizeof(_vel));
}

uint64_t
GlobalPositionPredictor::predict_position(uint64_t time_local, double *lat_pred, double *lon_pred, float *alt_pred) const
{
	int64_t dt = (int64_t)time_local - (int64_t)_time_last;
	if (dt < 0) {
		dt = 0;
	}
	float dt_s = dt * 1e-6f;
	add_vector_to_global_position(_lat, _lon, _vel[0] * dt_s, _vel[1] * dt_s, lat_pred, lon_pred);
	*alt_pred = _alt - _vel[2] * dt_s;
	return dt;
}

