#include <nuttx/config.h>

#include <conversion/rotation.h>
#include <drivers/drv_accel.h>
#include <drivers/drv_hrt.h>
#include <errno.h>
#include <fcntl.h>
#include <geo/geo.h>
#include <math.h>
#include <mathlib/mathlib.h>
#include <poll.h>
#include <uORB/topics/sensor_combined.h>
#include <stdio.h>
#include <string.h>
#include <systemlib/param/param.h>

#include "accel_calibration.hpp"

namespace calibration {

AccelCalibrator::AccelCalibrator() :
		dev_fd{0},
		inprogress{false},
		sensor_topic {0},
		poll_data {},
		accel_measure{},
		current_axis {-1},
		sampling_needed {true},
		calibrated_axes{} {
}

CALIBRATION_RESULT AccelCalibrator::init() {
	if (dev_fd == 0) {
		dev_fd = open(ACCEL_DEVICE_PATH, O_RDONLY);
	}
	accel_scale zero_scale = {
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		0.0f,
		1.0f
	};
	// Reset sensor scale
	if (ioctl(dev_fd, ACCELIOCSSCALE, (unsigned long int) &zero_scale) != 0) {
		return CALIBRATION_RESULT::SCALE_RESET_FAIL;
	}

	// Set up poller
	// Sensor combined is used to get rotated measurements, else we won't be able to assert that readings are equal to g
	sensor_topic = orb_subscribe(ORB_ID(sensor_combined));
	poll_data.fd = sensor_topic;
	poll_data.events = POLLIN;

	inprogress = true;

	return CALIBRATION_RESULT::SUCCESS;
}

CALIBRATION_RESULT AccelCalibrator::sample_axis() {

	// Do not allow running before init was run
	if (!inprogress) {
		return CALIBRATION_RESULT::FAIL;
	}

	current_axis = detect_orientation();
	if (current_axis < 0) {
		// TODO! Consider different error here
		return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
	}

	if (calibrated_axes[current_axis]) {
		return CALIBRATION_RESULT::AXIS_DONE_FAIL;
	}

	return CALIBRATION_RESULT::SUCCESS;
}

CALIBRATION_RESULT AccelCalibrator::calculate_and_save() {

	if (!inprogress || sampling_needed) {
		return CALIBRATION_RESULT::FAIL;
	}

	math::Matrix<3,3> accel_T;
	math::Vector<3> accel_offs;

	// Fill accel_T and accel_offs
	calculate_calibration_values(accel_T, accel_offs);

	// Main diagonal of the accel_T is responsible for the scaling, and other values - for "noisy" orientation
	// Thus we use only the values on the main diagonal
	accel_scale calibration_scale = {
		accel_offs(0),
		accel_T(0,0),
		accel_offs(1),
		accel_T(1,1),
		accel_offs(2),
		accel_T(2,2)
	};

	// Set calibration parameters
	if (param_set(param_find("SENS_ACC_SCALE"), &calibration_scale)) {
		return(CALIBRATION_RESULT::PARAMETER_SET_FAIL);
	}
	inprogress = false; // parameters are set, no need to roll back

	// Apply new calibration values to the driver
	if (ioctl(dev_fd, ACCELIOCSSCALE, (long unsigned int) &calibration_scale) != 0) {
		return(CALIBRATION_RESULT::SCALE_APPLY_FAIL);
	}

	// Save calibration parameters to EEPROM
	if (param_save_default() != 0) {
		return(CALIBRATION_RESULT::PARAMETER_DEFAULT_FAIL);
	}

	return CALIBRATION_RESULT::SUCCESS;
}

// Use the Exponential Movement Averaging algorithm to detect still position and find orientation
int AccelCalibrator::detect_orientation() {
	sensor_combined_s report;
	int res;

	unsigned int error_count = 0;
	unsigned int max_error_count = 1000;

	hrt_abstime prev_time = 0;
	hrt_abstime curr_time = 0;
	hrt_abstime still_start = 0;
	hrt_abstime still_end = 1;
	// 2 secs keeping still is ok
	hrt_abstime still_period = 2 * 1000 * 1000;

	// EMA period weighting coefficient
	float ema_len = 0.5f;
	// Measurement weight with regard to time delta
	float weight;
	float delta;

	// Averaged results
	float ema_accel[3] = {0.0f, 0.0f, 0.0f};
	// Weighted fluctuation values, we use max of (diminished previous results, weighted current result)
	float diminishing_deltas[3] = {0.0f, 0.0f, 0.0f};

	// Threshold meaning the sensor became still
	float still_threshold_sq = 0.25f * 0.25f;
	// Threshold meaning the sensor started to move
	float motion_threshold_sq = still_threshold_sq * 4;
	// Greater delta values will be cut before storing to diminishig_deltas array
	float extreme_cutoff_sq = still_threshold_sq * 8;
	bool stopped, moved;

	int timeout = 1000;
	// try no longer than 30 secs total
	hrt_abstime allowed_time = 30 * 1000 * 1000;
	const hrt_abstime end_time = hrt_absolute_time() + allowed_time;

	while (hrt_absolute_time() < end_time) {
		res = poll(&poll_data, 1, timeout);
		if (res == 1) {
			stopped = true;
			moved = false;

			orb_copy(ORB_ID(sensor_combined), sensor_topic, &report); // this should succeed
			if (prev_time != 0) { // There is no nice way to init it beforehand :-(
				prev_time = curr_time;
			}
			else {
				prev_time = report.timestamp;
			}
			curr_time = report.timestamp;
			// Doesn't seem right. With deltas >= 0.5 second, weight will become 1 or even bigger.
			// To the process it will basically mean discarding previous observations
			// But with low deltas we'll have weight almost 0, which means previous measurements 
			weight = (float(curr_time - prev_time) / 1000000.0f) / ema_len;
			for (int i = 0; i < 3; ++i) {
				// Shortened version of EMA(i) = EMA(i-1) * (1-w) + w * accelerometer_m_s2[i]
				delta = report.accelerometer_m_s2[i] - ema_accel[i];
				ema_accel[i] += delta * weight;

				// Preparing for threshold comparison
				delta *= delta;
				diminishing_deltas[i] *= 1.0f - weight;

				// Damping extreme values
				if (delta > extreme_cutoff_sq) {
					delta = extreme_cutoff_sq;
				}

				// Even diminished older values could be more extreme
				if (delta > diminishing_deltas[i]) {
					diminishing_deltas[i] = delta;
				}

				if (diminishing_deltas[i] > motion_threshold_sq) {
					moved = true;
					stopped = false;
				}
				else if (diminishing_deltas[i] >= still_threshold_sq) {
					stopped = false;
				}
			}
			if (stopped) {
                if (still_start != 0) {
                    if (curr_time >= still_end) {
                        break; // The only way to succeed
                    }
                }
                else {
                    still_start = curr_time;
                    still_end = curr_time + still_period;
                }
            }
            else if (moved) {
                still_start = 0;
            }
		}
		else {
			// Prevent successes after a moment of stillnes and some timeouts
			stopped = false;
			moved = true;
			if (++error_count > max_error_count) {
				return -1;
			}
		}
	}

	if (curr_time < still_end || !stopped) { // Didn't get enough still time before timeout
		return -1;
	}

	return detect_g(ema_accel);
}

int AccelCalibrator::detect_g(float accelarations[3]) {
	int res = -1;
	float accel_error_threshold = 5.0f;

	for (int i = 0; i < 3; ++i) {
		if (fabsf(fabsf(accelarations[i]) - CONSTANTS_ONE_G) < accel_error_threshold) {
			if (res != -1) { // several g axes
				res = -1;
				break;
			}
			if (accelarations[i] < 0) {
				res = i*2+1; // -g measurement found, maps to 0-5
			}
			else {
				res = i*2; // +g measurement found, maps to 0-5
			}
		}
		// Don't account for g deviations, just for sensor noise or imperfect board rotation
		else if (fabsf(accelarations[i]) >= accel_error_threshold) {
			res = -1; // Zero axis too far from zero
			break;
		}
	}
	return res;
}

CALIBRATION_RESULT AccelCalibrator::read_samples() {
	int sample_count = 2500;
	int max_error_count = 250;
	int timeout = 500;
	int error_count = 0, success_count = 0;
	int res;
	sensor_combined_s report;

	if (current_axis < 0 || current_axis > 5) {
		// No orientation has been detected
		return CALIBRATION_RESULT::FAIL;
	}

	while (success_count < sample_count && error_count <= max_error_count) {
		res = poll(&poll_data, 1, timeout);
		if (res == 1) {
			if (orb_copy(ORB_ID(sensor_combined), sensor_topic, &report) == 0) {
				++success_count;
				accel_measure[current_axis][0] += report.accelerometer_m_s2[0];
				accel_measure[current_axis][1] += report.accelerometer_m_s2[1];
				accel_measure[current_axis][2] += report.accelerometer_m_s2[2];
			}
			else {
				++error_count;
			}
		}
		else { // res == 0 - timeout, res < 0 - errors, res > 1 - most probably, corrupted memory.
			++error_count;
			printf("Kuso! Poll error! Return: %d, errno: %d, errcnt: %d, success: %d\n", res, errno, error_count, success_count); // TODO! Debug output. "Kuso!" in Japanese is roughly equivalent to "shit!"
		}
	}
	if (error_count > max_error_count) {
		current_axis = -1;
		return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
	}

	for (int i = 0; i < 3; ++i) {
		accel_measure[current_axis][i] /= success_count;
	}

	calibrated_axes[current_axis] = true;
	current_axis = -1;
	bool tmp = false;
	for (int i = 0; i < 6; ++i) {
		if (!calibrated_axes[i]) {
			tmp = true;
			break;
		}
	}
	sampling_needed = tmp;
	return CALIBRATION_RESULT::SUCCESS;
}

/* Calibration calculations:
 * We assume that measured values are g-accelerations and zeros but with an offset and with a transformation: rotation (user doesn't hold the drone perfectly) and scaling.
 * Sensor scales as follows: (measurement - offset) * scaling. Thus we need to adjust our equation to match this logic.
 * Therefore we get:
 * [g, 0, 0]
 * [-g, 0, 0] = TransformM * (MeasurementsM - offsets)
 * [0, g, 0]
 * ...
 * MeasurementsM is 6x3 matrix.
 * But TransformM needs to be 3x3 only, so we use every other reference measurement - only with positive g-s.
 * So, first we calculate offsets as measurement on the respective axis averaged of two samples: for g and -g scenario.
 * Offsets[i] = AVG(MeasurementM[i*2][i], MeasurementM[i*2+1][i]) - sample i*2 and i*2 + 1 - when g and -g are on i-th axis. The second subscript - number of the axis.
 * Then we apply offsets to the measurements on each axis separately: OffMeasurements[*][i] = MeasurementsM[*][i] - Offsets[i]
 * Then we use only every other row of the OffMeasurements - 0th, 2nd and 4th which correspond to the +g measurements.
 * PositiveMeasurements[i][*] = OffMeasurements[i*2][*]
 * TransformM * PositiveMeasurements = ExpectedValues
 * TransformM = ExpectedValues * inv(PositiveMeasurements)
 * But ExpectedValues is equal to just:
 * [g, 0, 0]
 * [0, g, 0]
 * [0, 0, g]
 * which is just g scalar.
 * TransformM = g * inv(PositiveMeasurements)
 */
void AccelCalibrator::calculate_calibration_values(math::Matrix<3,3> &accel_T, math::Vector<3> &accel_offs) {
	// calculate offsets as average of respective +g and -g measurements
	for (int i = 0; i < 3; ++i) {
		accel_offs(i) = (accel_measure[i * 2][i] + accel_measure[i * 2 + 1][i]) / 2;
	}

	// fill known value matrix for linear equations system
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			accel_T(i,j) = accel_measure[i*2][j] - accel_offs(j);
		}
	}

	accel_T = accel_T.inversed(); // unfortunately, .inversed() method ignores possible errors

	// In fact, a matrix multiplication should have been here, but the right matrix
	// consists only of zeroes and g constants on the main diagonal
	accel_T *= CONSTANTS_ONE_G;

	rotate_transformation(accel_T, accel_offs);
}

/* Logic is as follows: our input accel_T and accel_offs are in fact some kind of rotated transformations,
 * as they were calculated based on rotated sensor data. We need to find some kind of transformation and
 * offsets, because drivers + sensors.cpp first apply scaling and offsets and only then - the rotation.
 * accel_T * (rotation * data_raw - accel_offsets) = reference_values - what we have now.
 * rotation * res_T * (data_raw - res_offsets) = reference_values - what we want to happen.
 * Thus:
 * accel_T * (rotation * data_raw - accel_offsets) = rotation * res_T * (data_raw - res_offsets)
 * inv(rot) * accel_T * (rotation * data_raw - accel_offsets) = res_T * (data_raw - res_offsets)
 * inv(rot) * accel_T * rotation * data_raw - inv(rot) * accel_T * accel_offsets = res_T * data_raw - res_T * res_offsets
 * which will be true if
 * res_T = inv(rot) * accel_T * rotation
 * and
 * accel_offsets = rot * res_offsets
 * inv(rot) * accel_T * rotation * data_raw - inv(rot) * accel_T * rot * res_offsets = res_T * data_raw - res_T * res_offsets
 * res_T * data_raw - res_T * res_offsets = res_T * data_raw - res_T * res_offsets
 * Thus:
 * res_T = inv(rot) * accel_T * rotation
 * res_offsets = inv(rot) * accel_offsets
 */
void AccelCalibrator::rotate_transformation(math::Matrix<3,3> &accel_T, math::Vector<3> &accel_offs) {
	int board_rotation;
	param_get(param_find("SENS_BOARD_ROT"), &board_rotation);
	math::Matrix<3,3> rotation_matrix;
	get_rot_matrix((Rotation) board_rotation, &rotation_matrix);
	// For rotation matrices transposing and inverting gives the same result, but transposing is faster
	math::Matrix<3,3> inverted_rotation = rotation_matrix.transposed();

	accel_offs = inverted_rotation * accel_offs;
	accel_T = inverted_rotation * accel_T * rotation_matrix;
}

void AccelCalibrator::restore() {
	accel_scale saved_scale;
	if (inprogress && !param_get(param_find("SENS_ACC_SCALE"), &saved_scale)) {
		if (dev_fd == 0) {
			dev_fd = open(ACCEL_DEVICE_PATH, O_RDONLY);
		}
		ioctl(dev_fd, ACCELIOCSSCALE, (unsigned long int) &saved_scale);
	}
}

AccelCalibrator::~AccelCalibrator() {
	restore(); // Consider calling the method elsewhere... it is possible that scale has changed before destructor was invoked.
	if (dev_fd > 0) {
		close(dev_fd);
	}
	if (sensor_topic != 0) {
		close(sensor_topic);
	}
}

} // End calibration namespace
