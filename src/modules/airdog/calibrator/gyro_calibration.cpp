#include "gyro_calibration.hpp"

#include <drivers/drv_gyro.h> // report structure and ioctl commands
#include <errno.h> // errno to check for poll errors
#include <fcntl.h> // open
#include <math.h> // isfinite
#include <poll.h>
#include <stdio.h> // close
#include <systemlib/param/param.h>
#include <uORB/uORB.h>

namespace calibration {

// polls the sensor and calculates the average offset. Returns true on success, false on failure
bool sample_offsets (gyro_scale &calibration_scale, const unsigned int sample_count, const unsigned int max_error_count, const unsigned int timeout);

CALIBRATION_RESULT do_gyro_calibration(const unsigned int sample_count, const unsigned int max_error_count, const unsigned int timeout) {

	gyro_scale calibration_scale = {
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		0.0f,
		1.0f
	};

	// reset all offsets to zero and all scales to one
	// Beware! This only works because sensors.cpp doesn't check if parameters are different from driver settings.
	// TODO! consider to check if we can get at least one sample before resetting the calibration
	int fd = open(GYRO_DEVICE_PATH, O_RDONLY);
	if (ioctl(fd, GYROIOCSSCALE, (long unsigned int)&calibration_scale) != 0) {
		close(fd);
		return CALIBRATION_RESULT::SCALE_RESET_FAIL;
	}

	// calculate offsets
	if (!sample_offsets(calibration_scale, sample_count, max_error_count, timeout)) {
		close(fd);
		return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
	}

	// set offset and scale parameters. Scale parameters reset to 1, but that's the number we pass to the sensor driver.
	// TODO! migrate to single structure. Currently only calibration and drivers use these and both convert to struct.
	if (param_set(param_find("SENS_GYRO_SCALE"), &calibration_scale)) {
		close(fd);
		return CALIBRATION_RESULT::PARAMETER_SET_FAIL;
	}

	// applying the new values
	// Unless we turn off sensors.cpp resets, parameter values override GYROIOCSSCALE, so parameters need to be changed first.
	if (ioctl(fd, GYROIOCSSCALE, (long unsigned int)&calibration_scale) != 0) {
		close(fd);
		return CALIBRATION_RESULT::SCALE_APPLY_FAIL;
	}
	close(fd);

	if (param_save_default() != 0) {
		return CALIBRATION_RESULT::PARAMETER_DEFAULT_FAIL;
	}

	return CALIBRATION_RESULT::SUCCESS;
}

bool sample_offsets (gyro_scale &calibration_scale, unsigned int sample_count, unsigned int max_error_count, unsigned int timeout)
{
	// set up the poller
	int gyro_topic = orb_subscribe(ORB_ID(sensor_gyro0));
	gyro_report report;
	pollfd poll_data;
	poll_data.fd = gyro_topic;
	poll_data.events = POLLIN;

	int res = 0;

	// average polls until enough samples are collected or too many errors
	unsigned int success_count = 0;
	unsigned int error_count = 0;
	while (success_count < sample_count && error_count <= max_error_count) {
		// poll expects an array of length 1, but single pointer will work too
		res = poll(&poll_data, 1, timeout);
		if (res == 1) {
			if (orb_copy(ORB_ID(sensor_gyro0), gyro_topic, &report) == 0) {
				++success_count;
				calibration_scale.x_offset += report.x;
				calibration_scale.y_offset += report.y;
				calibration_scale.z_offset += report.z;
			}
			else {
				++error_count;
			}
		}
		else { // res == 0 - timeout, res < 0 - errors, res > 1 - most probably, corrupted memory.
			++error_count;
			printf("Kuso! Poll error! Return: %d, errno: %d, errcnt: %d\n", res, errno, error_count); // TODO! Debug output. "Kuso!" in Japanese is roughly equivalent to "shit!"
		}

	}
	close(gyro_topic);

	// averaging all the samples
	calibration_scale.x_offset /= sample_count;
	calibration_scale.y_offset /= sample_count;
	calibration_scale.z_offset /= sample_count;

	// everything went fine
	if (error_count <= max_error_count && isfinite(calibration_scale.x_offset)
			&& isfinite(calibration_scale.y_offset) && isfinite(calibration_scale.z_offset) ) {
		return true;
	}
	// too many errors or incorrect value in any of the offsets
	return false;
}

} // End namespace
