#pragma once

#include "calibration_commons.hpp"

namespace calibration {

/*
 * Main gyro calibration procedure. Scale calibration is not implemented.
 * sample_count - number of samples to average when calibrating offsets. Default: 5000. Make it positive or I will shoot you.
 * max_error_count - number of errors tolerated. Polling will return error if error count gets lager than this parameter. Default: 1000.
 * timeout - timeout for each poll request in ms. Worst case process will hang for timeout*(max_error_count + 1) ms. So be considerate. Default: 1000.
 * For return values - see calibration::CALIBRATION_RESULT enum.
 */
CALIBRATION_RESULT do_gyro_calibration(const unsigned int sample_count=5000, const unsigned int max_error_count=1000, const int timeout=1000);

} // End namespace

