#pragma once

#include <mathlib/mathlib.h>
#include <poll.h>

#include "calibration_commons.hpp"

namespace calibration {

/**
 * Main class for accelerometer calibration.
 * General procedure: call init() method.
 * While sampling_needed do:
 * 	sample_axis method. If the result is axis done - sample again,
 * 	if the result is data_error - fail, if success then read_samples method.
 * end.
 * If up till now no error has occurred then call calculate_and_save method.
 * Try to keep the scope of the object as small as possible -
 * because destructor cleans up any scale resets in case of an unfinished calibration.
 */
class AccelCalibrator {
	private:
		int dev_fd; // common file descriptor for accel driver communication
		bool inprogress; // internal flag indicating that calibration process has been started and cleanup will be necessary
		int sensor_topic; // common topic for polling
		pollfd poll_data; // common poll structure
		float accel_measure[6][3]; // averaged measurements when reading the axes: x+, x-, y+, y-, z+, z-
		int current_axis; // axis waiting to be sampled

		// Calculate correct scales and offsets based on accel_measure data. Sets accel_T and accel_offs
		void calculate_calibration_values(math::Matrix<3,3> &accel_T, math::Vector<3> &accel_offs);
		// Try to detect oriented position of the sensor. Returns axis number for calibrated_axes or -1 in case of error
		int detect_orientation();
		// Determine which of the axis and in which direction was sampled. Returns axis number for calibrated_axes or -1 in case of error
		int detect_g(float accelerations[3]);
		// Restore calibration values from parameter storage
		void restore();
		// Perform calibration transformation and offset rotation to match raw sensor data
		void rotate_transformation(math::Matrix<3,3> &accel_T, math::Vector<3> &accel_offs);
	public:
		bool sampling_needed; // Flag indicating that one should continue to sample axis
		bool calibrated_axes[6]; // Indicates if axis has been sampled: x+, x-, y+, y-, z+, z-

		AccelCalibrator();
		// Initialize sensor and prepare for calibration
		CALIBRATION_RESULT init();
		// Collect sample for one axis. Should be called while sample_needed is true
		CALIBRATION_RESULT sample_axis();
		// Read and average samples into the accel_measure array. Current_axis must be set via sample_axis
		CALIBRATION_RESULT read_samples();
		// When enough samples has been collected, calculate actual scaling and offsets
		CALIBRATION_RESULT calculate_and_save();
		// Destructor. Resets sensor scales to parameter value if calibration didn't finish.
		~AccelCalibrator();
};

} // End calibration namespace
