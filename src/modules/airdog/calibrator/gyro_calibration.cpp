#include <nuttx/config.h>

#include <drivers/drv_gyro.h> // ioctl commands
#include <drivers/calibration/calibration.hpp>
#include <stdio.h> // close
#include <fcntl.h> // open
#include <math.h> // isfinite
#include <poll.h>
#include <errno.h> // errno to check for poll errors
#include <systemlib/param/param.h>
#include <uORB/uORB.h>

#include "gyro_calibration.hpp"

namespace calibration {
// TODO! Implement scale reset in case of an error
// polls the sensor and calculates the average offset. Returns true on success, false on failure
bool sample_offsets (gyro_calibration_s &calibration, const unsigned int sample_count, const unsigned int max_error_count, const int timeout);

CALIBRATION_RESULT do_gyro_calibration(const unsigned int sample_count, const unsigned int max_error_count, const int timeout) {

	gyro_calibration_s calibration;

	// reset all offsets to zero and all scales to one
	// Beware! This only works because sensors.cpp doesn't check if parameters are different from driver settings.
	int fd = open(GYRO_DEVICE_PATH, O_RDONLY);
	if (ioctl(fd, GYROIOCSSCALE, (long unsigned int)&calibration) != 0) {
		close(fd);
		return CALIBRATION_RESULT::SCALE_RESET_FAIL;
	}

	// calculate offsets
	if (!sample_offsets(calibration, sample_count, max_error_count, timeout)) {
		close(fd);
		return CALIBRATION_RESULT::SENSOR_DATA_FAIL;
	}

	// Fill calibration conditions before setting parameters, errors are ignored
	fill_calibration_conditions(&calibration);
	// set offset and scale parameters. Scale parameters reset to 1, but that's the number we pass to the sensor driver.
	if (!set_calibration_parameters(calibration)) {
		close(fd);
		return CALIBRATION_RESULT::PARAMETER_SET_FAIL;
	}

	// applying the new values
	// Unless we turn off sensors.cpp resets, parameter values override GYROIOCSSCALE, so parameters need to be changed first.
	if (ioctl(fd, GYROIOCSSCALE, (long unsigned int)&calibration) != 0) {
		close(fd);
		return CALIBRATION_RESULT::SCALE_APPLY_FAIL;
	}
	close(fd);

	if (param_save_default() != 0) {
		return CALIBRATION_RESULT::PARAMETER_DEFAULT_FAIL;
	}

	return CALIBRATION_RESULT::SUCCESS;
}

bool sample_offsets (gyro_calibration_s &calibration, const unsigned int sample_count, const unsigned int max_error_count, const int timeout)
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
				calibration.offsets(0) += report.x;
				calibration.offsets(1) += report.y;
				calibration.offsets(2) += report.z;
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
	calibration.offsets /= sample_count;
	// everything went fine
	if (error_count <= max_error_count && isfinite(calibration.offsets(0))
			&& isfinite(calibration.offsets(1)) && isfinite(calibration.offsets(2)) ) {
		return true;
	}
	// too many errors or incorrect value in any of the offsets
	return false;
}

} // End calibration namespace
