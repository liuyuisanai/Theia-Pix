#pragma once

#include <mathlib/mathlib.h>
#include <systemlib/param/param.h>

struct __EXPORT calibration_values_s {
	math::Vector<3> offsets;
	math::Vector<3> scales;
	inline calibration_values_s() {
		scales.set(1.0f);
	}
};

/**
 * accel scaling factors; Vout = Vscale * (Vin + Voffset)
 * Inheritance enables overloading for parameter saving
 */
struct __EXPORT accel_calibration_s : calibration_values_s {};

inline __EXPORT bool set_calibration_parameters (const char* param_names[6] , const calibration_values_s &calibration) {
	for (int i = 0; i < 3; ++i) {
		if (param_set(param_find(param_names[i]), &(calibration.offsets(i)))) {
			return false;
		}
		if (param_set(param_find(param_names[i+3]), &(calibration.scales(i)))) {
			return false;
		}
	}
	return true;
}

inline __EXPORT bool set_calibration_parameters (const accel_calibration_s &accel_calibration) {
	static const char *names[6] = {"SENS_ACC_XOFF", "SENS_ACC_YOFF", "SENS_ACC_ZOFF",
			"SENS_ACC_XSCALE", "SENS_ACC_YSCALE", "SENS_ACC_ZSCALE"};
	return set_calibration_parameters(names, accel_calibration);
}

inline __EXPORT bool get_calibration_parameters (const char* param_names[6] , calibration_values_s *calibration) {
	for (int i = 0; i < 3; ++i) {
		if (param_get(param_find(param_names[i]), &(calibration->offsets(i)))) {
			return false;
		}
		if (param_get(param_find(param_names[i+3]), &(calibration->scales(i)))) {
			return false;
		}
	}
	return true;
}

inline __EXPORT bool get_calibration_parameters (accel_calibration_s *accel_calibration) {
	static const char *names[6] = {"SENS_ACC_XOFF", "SENS_ACC_YOFF", "SENS_ACC_ZOFF",
			"SENS_ACC_XSCALE", "SENS_ACC_YSCALE", "SENS_ACC_ZSCALE"};
	return get_calibration_parameters(names, accel_calibration);
}
