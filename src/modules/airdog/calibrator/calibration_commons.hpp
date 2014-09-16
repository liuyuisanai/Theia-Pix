#pragma once

#include <stdint.h>

namespace calibration {

// 0 - done, other codes - errors.
enum class CALIBRATION_RESULT : uint8_t {
	SUCCESS = 0, // calibration done
	FAIL = 1, // general error code
	SCALE_RESET_FAIL = 2, // failed to set sensor scale
	SCALE_APPLY_FAIL = 3, // failed to apply new values. Different from initial reset, because parameters are already done
	SENSOR_DATA_FAIL = 4, // failed to get sane data from sensor
	PARAMETER_DEFAULT_FAIL = 5, // failed to save parameters to EEPROM
	PARAMETER_SET_FAIL = 6, // failed to set scale parameters
	SCALE_READ_FAIL = 7 // failed to read scale values
};

} // End namespace

