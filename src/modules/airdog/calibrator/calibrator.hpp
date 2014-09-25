/* Calibration functions.
 */

#pragma once

namespace calibration {
/* Starts gyroscope calibration
 * sample_count - number of samples to average when calibrating offsets. Default: 5000. Make it positive or I will shoot you.
 * max_error_count - number of errors tolerated. Polling will return error if error count gets lager than this parameter. Default: 1000.
 * timeout - timeout for each poll request in ms. Worst case process will hang for timeout*(max_error_count + 1) ms. So be considerate. Default: 1000.
 * @return true if calibration was successful, false otherwise
 */
bool calibrate_gyroscope(const unsigned int sample_count=5000,
						const unsigned int max_error_count=1000,
						const int timeout=1000);

/* Starts magnetometer calibration procedure.
 * Samples will be equally spaced during the calibration, but total_time/sample_count should not be higher than sensor update rate.
 * sample_count - number of samples to be taken during the calibration time. Default: 6000
 * max_error_count - allowed number of errors while polling the sensor. Default: 200
 * total_time - total time in ms for the measurement. Default: 60000
 * poll_timeout_gap - gap in ms between orb publishing interval and timeout on poll requests. Default 5
 * @return true if calibration was successful, false otherwise
 */
bool calibrate_magnetometer(unsigned int sample_count=6000,
							unsigned int max_error_count=200,
							unsigned int total_time=60000,
							int poll_timeout_gap=5);
} // End calibration namespace
