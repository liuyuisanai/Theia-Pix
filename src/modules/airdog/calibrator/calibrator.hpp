/* Calibration functions.
 */

#pragma once

namespace calibration {
/* Starts gyroscope calibration procedure.
 * mavlink_fd - if not zero, then messages will be sent via mavlink too
 * sample_count - number of samples to average when calibrating offsets. Default: 5000. Make it positive or I will shoot you.
 * max_error_count - number of errors tolerated. Polling will return error if error count gets lager than this parameter. Default: 100.
 * timeout - timeout for each poll request in ms. Worst case process will hang for timeout*(max_error_count + 1) ms. So be considerate. Default: 200.
 * @return true if calibration was successful, false otherwise
 */
__EXPORT bool calibrate_gyroscope(int mavlink_fd=0, const unsigned int sample_count=5000,
						const unsigned int max_error_count=100,
						const int timeout=200);

/* Starts magnetometer calibration procedure.
 * mavlink_fd - if not zero, then messages will be sent via mavlink too
 * Samples will be equally spaced during the calibration, but total_time/sample_count should not be higher than sensor update rate.
 * sample_count - number of samples to be taken during the calibration time. Default: 6000
 * max_error_count - allowed number of errors while polling the sensor. Default: 200
 * total_time - total time in ms for the measurement. Default: 60000
 * poll_timeout_gap - gap in ms between orb publishing interval and timeout on poll requests. Default 5
 * @return true if calibration was successful, false otherwise
 */
__EXPORT bool calibrate_magnetometer(int mavlink_fd=0, unsigned int sample_count=6000,
							unsigned int max_error_count=200,
							unsigned int total_time=60000,
							int poll_timeout_gap=5);

/* Starts accelerometer calibration procedure.
 * Requires the user to rotate the object orthogonal to the gravitational field.
 * mavlink_fd - if not zero, then messages will be sent via mavlink too
 * @return true if calibration was successful, false otherwise
 */
__EXPORT bool calibrate_accelerometer(int mavlink_fd=0);

/* Checks if the copter is in rest state (aka standing still)
 * timeout - timeout after which the function will consider copter to be moving
 * minimal_time - minimal time in ms that copter has to stand still, if 0 then only one sample will be read
 * mavlink_fd - if not zero, then warnings will be sent trough specified mavlink channel
 * threshold - in m/s - the function will be true only if difference between measurement and g will be less
 * @return true if the copter is still, false - if copter is moving or accel fails self-test
 */
__EXPORT bool check_resting_state(unsigned int timeout, unsigned int minimal_time = 0, int mavlink_fd = 0, float threshold = 0.1f);
} // End calibration namespace
