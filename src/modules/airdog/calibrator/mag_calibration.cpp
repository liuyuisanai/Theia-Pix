#include <nuttx/config.h>

#include <drivers/drv_mag.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <systemlib/param/param.h>
#include <uORB/uORB.h>

#include "mag_calibration.hpp"

namespace calibration {

struct axis_stat_s {
	float sumplain;
	float sumsq;
	float sumcube;
};
struct sample_stat_s {
	axis_stat_s x, y, z;
	float xy_sum, xz_sum, yz_sum;
	float x2y_sum, x2z_sum, y2x_sum, y2z_sum, z2x_sum, z2y_sum;
	unsigned int sample_count;
};

// Sample offsets and provide stats required for sphere fit
bool sample_offsets (sample_stat_s &res_stats, unsigned int sample_count, unsigned int max_error_count, unsigned int total_time, int poll_timeout_gap);

// Calculate sums, sum squares and other stats for fit_sphere
inline void calc_stats(mag_report &report, sample_stat_s &res_stats);

// Average collected stats by sample count
inline void average_stats(sample_stat_s &res_stats);

// Original method trying to fit the samples into a sphere
bool sphere_fit_least_squares(sample_stat_s res_stats, float &sphere_x, float &sphere_y, float &sphere_z);

/**
 * Try to restore sensor to the state before the calibration
 * If reset_scale is true, then magnetometer scale will be reset to parameter values
 */
inline void cleanup(int fd, bool reset_scale) {
	mag_scale saved_scale;
	if (reset_scale && !param_get(param_find("SENS_MAG_SCALE"), &saved_scale)) {
		if (fd <= 0) {
			fd = open(MAG_DEVICE_PATH, O_RDONLY);
		}
		// Errors are ignored - there is not much we can do about them.
		ioctl(fd, MAGIOCSSCALE, (long unsigned int) &saved_scale);
	}
	if (fd > 0) {
		close(fd);
	}
};

CALIBRATION_RESULT do_mag_builtin_calibration() {
	int fd = open(MAG_DEVICE_PATH, O_RDONLY);
	if (ioctl(fd, MAGIOCCALIBRATE, fd) != 0) {
		// This still is not critical - it is possible internal calibration is not available
		// Internal calibration routines should reset calibration to previous values in case of failure
		mag_scale zero_scale = {
			0.0f,
			1.0f,
			0.0f,
			1.0f,
			0.0f,
			1.0f
		};
		if (ioctl(fd, MAGIOCSSCALE, (unsigned long int) &zero_scale) != 0) {
			// This is critical, but still no need to reset the calibration to previous values
			cleanup(fd,false);
			return (CALIBRATION_RESULT::SCALE_RESET_FAIL);
		}
		//TODO: Consider returning a warning here.
	}
	cleanup(fd, false);
	return (CALIBRATION_RESULT::SUCCESS);
}

CALIBRATION_RESULT do_mag_offset_calibration(unsigned int sample_count, unsigned int max_error_count, unsigned int total_time, int poll_timeout_gap) {

	mag_scale calibration_scale;
	int fd;
	sample_stat_s res_stats = {};

	// Get calibration values
	fd = open(MAG_DEVICE_PATH, O_RDONLY);
	if (ioctl(fd, MAGIOCGSCALE, (long unsigned int) &calibration_scale) != 0) {
		cleanup(fd, true);
		return(CALIBRATION_RESULT::SCALE_READ_FAIL);
	}

	// Collect samples
	if (!sample_offsets(res_stats, sample_count, max_error_count, total_time, poll_timeout_gap)) {
		cleanup(fd, true);
		return(CALIBRATION_RESULT::SENSOR_DATA_FAIL);
	}

	sphere_fit_least_squares(res_stats, calibration_scale.x_offset, calibration_scale.y_offset, calibration_scale.z_offset);

	// Set calibration parameters
	if (param_set(param_find("SENS_MAG_SCALE"), &calibration_scale)) {
		cleanup(fd, true);
		return(CALIBRATION_RESULT::PARAMETER_SET_FAIL);
	}

	// Apply new calibration values to the driver
	if (ioctl(fd, MAGIOCSSCALE, (long unsigned int) &calibration_scale) != 0) {
		cleanup(fd, false);
		return(CALIBRATION_RESULT::SCALE_APPLY_FAIL);
	}
	close(fd);

	// Save calibration parameters to EEPROM
	if (param_save_default() != 0) {
		return(CALIBRATION_RESULT::PARAMETER_DEFAULT_FAIL);
	}

	return(CALIBRATION_RESULT::SUCCESS);
}

bool sample_offsets(sample_stat_s &res_stats, unsigned int sample_count, unsigned int max_error_count, unsigned int total_time, int poll_timeout_gap) {
	unsigned int interval = total_time / sample_count; // should be greater than 10. Sensor updates at 100-150 Hz
	int timeout = interval + poll_timeout_gap; // give it a reasonable amount of time to update the topic

	// set up the poller
	int mag_topic = orb_subscribe(ORB_ID(sensor_mag0));
	mag_report report;
	pollfd poll_data;
	poll_data.fd = mag_topic;
	poll_data.events = POLLIN;

	int res = 0;

	// collect the stats for fit_sphere until the time is up
	
	unsigned int success_count = 0;
	unsigned int error_count = 0;
	orb_set_interval(mag_topic, interval); // limit the topic update frequency
	while (success_count < sample_count && error_count <= max_error_count) {
		// poll expects an array of length 1, but single pointer will work too
		res = poll(&poll_data, 1, timeout);
		if (res == 1) {
			if (orb_copy(ORB_ID(sensor_mag0), mag_topic, &report) == 0) {
				++success_count;
				calc_stats(report, res_stats);				
			}
			else {
				++error_count;
			}
		}
		else { // res == 0 - timeout, res < 0 - errors, res > 1 - most probably, corrupted memory.
			++error_count;
			printf("Kuso! Poll error! Return: %d, errno: %d, errcnt: %d, success: %d\n", res, errno, error_count, success_count); // TODO! Debug output. "Kuso!" in Japanese is roughly equivalent to "shit!"
		}
	}
	close(mag_topic);

	res_stats.sample_count = success_count;
	average_stats(res_stats);
	return (error_count <= max_error_count);
}

inline void calc_stats(mag_report &report, sample_stat_s &res_stats) {
	float x2 = report.x * report.x;
	float y2 = report.y * report.y;
	float z2 = report.z * report.z;

	res_stats.x.sumplain += report.x;
	res_stats.x.sumsq += x2;
	res_stats.x.sumcube += x2 * report.x;

	res_stats.y.sumplain += report.y;
	res_stats.y.sumsq += y2;
	res_stats.y.sumcube += y2 * report.y;

	res_stats.z.sumplain += report.z;
	res_stats.z.sumsq += z2;
	res_stats.z.sumcube += z2 * report.z;

	res_stats.xy_sum += report.x * report.y;
	res_stats.xz_sum += report.x * report.z;
	res_stats.yz_sum += report.y * report.z;

	res_stats.x2y_sum += x2 * report.y;
	res_stats.x2z_sum += x2 * report.z;

	res_stats.y2x_sum += y2 * report.x;
	res_stats.y2z_sum += y2 * report.z;

	res_stats.z2x_sum += z2 * report.x;
	res_stats.z2y_sum += z2 * report.y;
}

inline void average_stats(sample_stat_s &res_stats) {
	res_stats.x.sumplain /= res_stats.sample_count;
	res_stats.x.sumsq /= res_stats.sample_count;
	res_stats.x.sumcube /= res_stats.sample_count;

	res_stats.y.sumplain /= res_stats.sample_count;
	res_stats.y.sumsq /= res_stats.sample_count;
	res_stats.y.sumcube /= res_stats.sample_count;

	res_stats.z.sumplain /= res_stats.sample_count;
	res_stats.z.sumsq /= res_stats.sample_count;
	res_stats.z.sumcube /= res_stats.sample_count;

	res_stats.xy_sum /= res_stats.sample_count;
	res_stats.xz_sum /= res_stats.sample_count;
	res_stats.yz_sum /= res_stats.sample_count;

	res_stats.x2y_sum /= res_stats.sample_count;
	res_stats.x2z_sum /= res_stats.sample_count;

	res_stats.y2x_sum /= res_stats.sample_count;
	res_stats.y2z_sum /= res_stats.sample_count;

	res_stats.z2x_sum /= res_stats.sample_count;
	res_stats.z2y_sum /= res_stats.sample_count;
}

bool sphere_fit_least_squares(sample_stat_s res_stats, float &sphere_x, float &sphere_y, float &sphere_z)
{
	//
	//Least Squares Fit a sphere A,B,C with radius squared Rsq to 3D data
	//
	//    P is a structure that has been computed with the data earlier.
	//    P.npoints is the number of elements; the length of X,Y,Z are identical.
	//    P's members are logically named.
	//
	//    X[n] is the x component of point n
	//    Y[n] is the y component of point n
	//    Z[n] is the z component of point n
	//
	//    A is the x coordiante of the sphere
	//    B is the y coordiante of the sphere
	//    C is the z coordiante of the sphere
	//    Rsq is the radius squared of the sphere.
	//
	//This method should converge; maybe 5-100 iterations or more.
	//

	// The parameters seem to be satisfactory
	const unsigned int max_iterations = 100;
	const float delta = 0.0f;

	//Reduction of multiplications
	float F0 = res_stats.x.sumsq + res_stats.y.sumsq + res_stats.z.sumsq;
	float F1 =  0.5f * F0;
	float F2 = -8.0f * (res_stats.x.sumcube + res_stats.y2x_sum + res_stats.z2x_sum);
	float F3 = -8.0f * (res_stats.x2y_sum + res_stats.y.sumcube + res_stats.z2y_sum);
	float F4 = -8.0f * (res_stats.x2z_sum + res_stats.y2z_sum + res_stats.z.sumcube);

	//Set initial conditions:
	float A = res_stats.x.sumplain;
	float B = res_stats.y.sumplain;
	float C = res_stats.z.sumplain;

	//First iteration computation:
	float A2 = A * A;
	float B2 = B * B;
	float C2 = C * C;
	float QS = A2 + B2 + C2;
	float QB = -2.0f * (A * res_stats.x.sumplain + B * res_stats.y.sumplain + C * res_stats.z.sumplain);

	//Set initial conditions:
	float Rsq = F0 + QB + QS;

	//First iteration computation:
	float Q0 = 0.5f * (QS - Rsq);
	float Q1 = F1 + Q0;
	float Q2 = 8.0f * (QS - Rsq + QB + F0);
	float aA, aB, aC, nA, nB, nC, dA, dB, dC;

	//Iterate N times, ignore stop condition.
	unsigned int n = 0;

	while (n < max_iterations) {
		n++;

		//Compute denominator:
		aA = Q2 + 16.0f * (A2 - 2.0f * A * res_stats.x.sumplain + res_stats.x.sumsq);
		aB = Q2 + 16.0f * (B2 - 2.0f * B * res_stats.y.sumplain + res_stats.y.sumsq);
		aC = Q2 + 16.0f * (C2 - 2.0f * C * res_stats.z.sumplain + res_stats.z.sumsq);
		aA = (fabsf(aA) < FLT_EPSILON) ? 1.0f : aA;
		aB = (fabsf(aB) < FLT_EPSILON) ? 1.0f : aB;
		aC = (fabsf(aC) < FLT_EPSILON) ? 1.0f : aC;

		//Compute next iteration
		nA = A - ((F2 + 16.0f * (B * res_stats.xy_sum + C * res_stats.xz_sum + res_stats.x.sumplain * (-A2 - Q0) + 
				A * (res_stats.x.sumsq + Q1 - C * res_stats.z.sumplain - B * res_stats.y.sumplain))) / aA);
		nB = B - ((F3 + 16.0f * (A * res_stats.xy_sum + C * res_stats.yz_sum + res_stats.y.sumplain * (-B2 - Q0) + 
				B * (res_stats.y.sumsq + Q1 - A * res_stats.x.sumplain - C * res_stats.z.sumplain))) / aB);
		nC = C - ((F4 + 16.0f * (A * res_stats.xz_sum + B * res_stats.yz_sum + res_stats.z.sumplain * (-C2 - Q0) + 
				C * (res_stats.z.sumsq + Q1 - A * res_stats.x.sumplain - B * res_stats.y.sumplain))) / aC);

		//Check for stop condition
		dA = (nA - A);
		dB = (nB - B);
		dC = (nC - C);

		if ((dA * dA + dB * dB + dC * dC) <= delta) { break; }

		//Compute next iteration's values
		A = nA;
		B = nB;
		C = nC;
		A2 = A * A;
		B2 = B * B;
		C2 = C * C;
		QS = A2 + B2 + C2;
		QB = -2.0f * (A * res_stats.x.sumplain + B * res_stats.y.sumplain + C * res_stats.z.sumplain);
		Rsq = F0 + QB + QS;
		Q0 = 0.5f * (QS - Rsq);
		Q1 = F1 + Q0;
		Q2 = 8.0f * (QS - Rsq + QB + F0);
	}

	sphere_x = A;
	sphere_y = B;
	sphere_z = C;
	// *sphere_radius = sqrtf(Rsq);

	return true;
}

} // End calibration namespace
