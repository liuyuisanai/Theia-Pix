#pragma once

#include <mathlib/mathlib.h>

/*
 * common sensor scaling factors; Vout = Vscale * (Vin + Voffset)
 */

struct __EXPORT calibration_values_s {
	math::Vector<3> offsets;
	math::Vector<3> scales;
	inline calibration_values_s() {
		scales.set(1.0f);
	}
};

/**
 * Inheritance enables overloading for parameter saving
 */
struct __EXPORT accel_calibration_s : calibration_values_s {};
struct __EXPORT gyro_calibration_s : calibration_values_s {};
struct __EXPORT mag_calibration_s : calibration_values_s {};

/*
 * Sets parameters specified in offset_params and scale_params from values in calibration argument
 */
__EXPORT bool set_calibration_parameters (const char* const offset_params[3], const char* const scale_params[3],
		const calibration_values_s &calibration);
/*
 * Helper function to set accelerometer calibration parameters
 */
__EXPORT bool set_calibration_parameters (const accel_calibration_s &accel_calibration);
/*
 * Helper function to set gyroscope calibration parameters
 */
__EXPORT bool set_calibration_parameters (const gyro_calibration_s &gyro_calibration);
/*
 * Helper function to set magnetometer calibration parameters
 */
__EXPORT bool set_calibration_parameters (const mag_calibration_s &mag_calibration);

/*
 * Gets parameters specified in offset_params and scale_params and stores them in calibration argument
 */
__EXPORT bool get_calibration_parameters (const char* const offset_params[3], const char* const scale_params[3],
		calibration_values_s *calibration);
/*
 * Helper function to get accelerometer calibration parameters
 */
__EXPORT bool get_calibration_parameters (accel_calibration_s *accel_calibration);
/*
 * Helper function to get gyroscope calibration parameters
 */
__EXPORT bool get_calibration_parameters (gyro_calibration_s *gyro_calibration);
/*
 * Helper function to get magnetometer calibration parameters
 */
__EXPORT bool get_calibration_parameters (mag_calibration_s *mag_calibration);

/*
 * Print calibration values from calibration argument to console
 * If mavlink_fd argument is nonzero, then sends mavlink_info message too
 */
__EXPORT void print_calibration(const calibration_values_s &calibration, int mavlink_fd = 0);
