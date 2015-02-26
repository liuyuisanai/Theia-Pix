#include <nuttx/config.h>

#include <mavlink/mavlink_log.h>
#include <stdio.h>
#include <systemlib/param/param.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/uORB.h>

#include "calibration.hpp"

bool set_calibration_parameters (const char* const offset_params[3], const char* const scale_params[3],
		const char * const date_param, const char * const temp_param, const calibration_values_s &calibration) {
	for (int i = 0; i < 3; ++i) {
		if (param_set(param_find(offset_params[i]), &(calibration.offsets(i)))) {
			return false;
		}
		if (param_set(param_find(scale_params[i]), &(calibration.scales(i)))) {
			return false;
		}
	}

	int gps_topic, sensors_topic;
	gps_topic = orb_subscribe(ORB_ID(vehicle_gps_position));
	sensors_topic = orb_subscribe(ORB_ID(sensor_combined));
	vehicle_gps_position_s gps_data;
	sensor_combined_s sensor_data;
	uint32_t gps_time_trimmed;

	// Should fail if GPS fix is not obtained yet
	if (orb_copy(ORB_ID(vehicle_gps_position), gps_topic, &gps_data) == 0) {
		gps_time_trimmed = gps_data.time_gps_usec / 1000000 / 60 / 60; // Convert to hours to avoid overflow longer
		param_set(param_find(date_param), &(gps_time_trimmed));
	}

	if (orb_copy(ORB_ID(sensor_combined), sensors_topic, &sensor_data) == 0) {
		param_set(param_find(temp_param), &(sensor_data.baro_temp_celcius));
	}
	close(gps_topic);
	close(sensors_topic);

	return true;
}

bool set_calibration_parameters (const accel_calibration_s &accel_calibration) {
	constexpr char *offset_params[3] = {"SENS_ACC_XOFF", "SENS_ACC_YOFF", "SENS_ACC_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_ACC_XSCALE", "SENS_ACC_YSCALE", "SENS_ACC_ZSCALE"};
	return set_calibration_parameters(offset_params, scale_params, "SENS_ACC_CDATE", "SENS_ACC_CTEMP", accel_calibration);
}

bool set_calibration_parameters (const gyro_calibration_s &gyro_calibration) {
	constexpr char *offset_params[3] = {"SENS_GYRO_XOFF", "SENS_GYRO_YOFF", "SENS_GYRO_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_GYRO_XSCALE", "SENS_GYRO_YSCALE", "SENS_GYRO_ZSCALE"};
	return set_calibration_parameters(offset_params, scale_params, "SENS_GYRO_CDATE", "SENS_GYRO_CTEMP", gyro_calibration);
}

bool set_calibration_parameters (const mag_calibration_s &mag_calibration) {
	constexpr char *offset_params[3] = {"SENS_MAG_XOFF", "SENS_MAG_YOFF", "SENS_MAG_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_MAG_XSCALE", "SENS_MAG_YSCALE", "SENS_MAG_ZSCALE"};
	return set_calibration_parameters(offset_params, scale_params, "SENS_MAG_CDATE", "SENS_MAG_CTEMP", mag_calibration);
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

bool get_calibration_parameters (gyro_calibration_s *gyro_calibration) {
	constexpr char *offset_params[3] = {"SENS_GYRO_XOFF", "SENS_GYRO_YOFF", "SENS_GYRO_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_GYRO_XSCALE", "SENS_GYRO_YSCALE", "SENS_GYRO_ZSCALE"};
	return get_calibration_parameters(offset_params, scale_params, gyro_calibration);
}

bool get_calibration_parameters (mag_calibration_s *mag_calibration) {
	constexpr char *offset_params[3] = {"SENS_MAG_XOFF", "SENS_MAG_YOFF", "SENS_MAG_ZOFF"};
	constexpr char *scale_params[3] = {"SENS_MAG_XSCALE", "SENS_MAG_YSCALE", "SENS_MAG_ZSCALE"};
	return get_calibration_parameters(offset_params, scale_params, mag_calibration);
}

void print_calibration(calibration_values_s calibration, int mavlink_fd) {
	printf("Offsets: X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\nScales:  X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n",
			(double) calibration.offsets(0), (double) calibration.offsets(1), (double) calibration.offsets(2),
			(double) calibration.scales(0), (double) calibration.scales(1), (double) calibration.scales(2));
	if (mavlink_fd != 0) {
		mavlink_log_info(mavlink_fd, "Offsets: "
				"X: % 9.6f, Y: % 9.6f, Z: % 9.6f.",
				(double) calibration.offsets(0), (double) calibration.offsets(1), (double) calibration.offsets(2));
		mavlink_log_info(mavlink_fd, "Scales:  "
				"X: % 9.6f, Y: % 9.6f, Z: % 9.6f.",
				(double) calibration.scales(0), (double) calibration.scales(1), (double) calibration.scales(2));
	}
}
