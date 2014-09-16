/**
 * Magnetometer calibration.
 */

#pragma once

#include "calibration_commons.hpp"

namespace calibration {

/* Magnetometer internal calibration routines.
 * In case internal calibration is not present,
 * simply resets the calibration scales to 0.0 offsets and 1.0 scaling
 */
CALIBRATION_RESULT do_mag_builtin_calibration();

/* Main magnetometer calibration procedure. Builtin calibration should be called first!
 * Samples will be equally spaced during the calibration, but total_time/sample_count should not be higher than sensor update rate.
 * sample_count - number of samples to be taken during the calibration time
 * max_error_count - allowed number of errors while polling the sensor
 * total_time - total time in ms for the measurement
 * poll_timeout_gap - gap in ms between orb publishing interval and timeout on poll requests
 * See CALIBRATION_RESULT enum for possible return values
 */
CALIBRATION_RESULT do_mag_offset_calibration(unsigned int sample_count=6000, unsigned int max_error_count=200, unsigned int total_time=60000, int poll_timeout_gap=5);


} // End namespace
