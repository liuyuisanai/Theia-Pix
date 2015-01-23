#include <nuttx/config.h>

#include <systemlib/param/param.h>

#include "calibration.hpp"

bool set_calibration_parameters (const char* const offset_params[3], const char* const scale_params[3],
		const calibration_values_s &calibration) {
	for (int i = 0; i < 3; ++i) {
		if (param_set(param_find(offset_params[i]), &(calibration.offsets(i)))) {
			return false;
		}
		if (param_set(param_find(scale_params[i]), &(calibration.scales(i)))) {
			return false;
		}
	}
	return true;
}

bool set_calibration_parameters (const accel_calibration_s &accel_calibration) {
	constexpr char *offset_params[3] = {"SENS_ACC_XOFF", "SENS_ACC_YOFF", "SENS_ACC_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_ACC_XSCALE", "SENS_ACC_YSCALE", "SENS_ACC_ZSCALE"};
	return set_calibration_parameters(offset_params, scale_params, accel_calibration);
}

bool get_calibration_parameters (const char* const offset_params[3], const char* const scale_params[3],
		calibration_values_s *calibration) {
	for (int i = 0; i < 3; ++i) {
		if (param_get(param_find(offset_params[i]), &(calibration->offsets(i)))) {
			return false;
		}
		if (param_get(param_find(scale_params[i]), &(calibration->scales(i)))) {
			return false;
		}
	}
	return true;
}

bool get_calibration_parameters (accel_calibration_s *accel_calibration) {
	constexpr char *offset_params[3] = {"SENS_ACC_XOFF", "SENS_ACC_YOFF", "SENS_ACC_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_ACC_XSCALE", "SENS_ACC_YSCALE", "SENS_ACC_ZSCALE"};
	return get_calibration_parameters(offset_params, scale_params, accel_calibration);
}
