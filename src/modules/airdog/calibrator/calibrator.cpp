#include <drivers/drv_gyro.h> // ioctl commands
#include <drivers/drv_mag.h>
#include <fcntl.h> // open
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calibration_commons.hpp"
#include "gyro_calibration.hpp"
#include "mag_calibration.hpp"

// Execution messages
#define MSG_CALIBRATION_START "Starting %s calibration. Hold the system still.\n"
#define MSG_CALIBRATION_FINISH "Calibration finished with status: %d.\n"
#define MSG_CALIBRATION_USAGE "Usage: %s module_name\nmodule_name is one of accel, gyro, mag, baro, airspeed, rc, all\n" \
			"Advanced mode - gyro supports 3 parameters: sample count, max error count\n" \ 
			"and timeout in ms (defaults: 5000, 1000, 1000)\n"
#define MSG_CALIBRATION_NOT_IMPLEMENTED "Not supported yet. Sorry.\n"
#define MSG_CALIBRATION_WRONG_MODULE "Unknown module name \"%s\". Try accel, gyro, mag, baro, airspeed, rc, all\n"
#define MSG_CALIBRATION_RESULTS	"Result offsets: X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\nResult scales:  X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n"
#define MSG_CALIBRATION_GYRO_WRONG_PARAM "0 or 3 parameters required.\nValid ranges for samples 1-1000000, for errors 0-5000, for timeout 2-10000.\n"
#define MSG_CALIBRATION_MAG_WRONG_PARAM "0 or 4 parameters required.\nValid ranges for samples 100-total_time/5, for errors 0-sample_count,\nfor time 1-1000000, for gap 1-100.\n"
#define MSG_CALIBRATION_START_PARAM	"Starting %s calibration with parameters:\nsamples = %d, max errors = %d, timeout = %d.\n"
#define MSG_CALIBRATION_MAG_ROTATE "Sampling magnetometer offsets. Do a full rotation around each axis.\n"

using namespace calibration;

extern "C" __EXPORT int calibrator_main(int argc, char ** argv)
{
	char* sensname;
	CALIBRATION_RESULT res = CALIBRATION_RESULT::FAIL;

	const char* errors[] = { // allows to index by (error code)
		"No errors reported.\n", // code = 0 = SUCCESS
		"Calibration failed.\n", // code = 1 = FAIL
		"Failed to reset sensor scale.\n", // code = 2 = SCALE_RESET_FAIL
		"Failed to apply sensor scale.\n", // code = 3 = SCALE_APPLY_FAIL
		"Failed to get sane data from sensor.\n", // code = 4 = SENSOR_DATA_FAIL
		"Failed to save parameters to EEPROM.\n", // code = 5 = PARAMETER_DEFAULT_FAIL
		"Failed to set scaling parameters.\n", // code = 6 = PARAMETER_SET_FAIL
		"Failed to read sensor scale.\n" // code = 7 = SCALE_READ_FAIL
	};
	const size_t errors_size = sizeof(errors) / sizeof(*errors);

	if (argc < 2 || argc > 6) {
		fprintf(stderr, MSG_CALIBRATION_USAGE, argv[0]);
		return 1;
	}
	sensname = argv[1];

	// TODO! consider possibility of merging the if-s.
	if (strcmp(sensname, "accel") == 0) {
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
	}
	else if (strcmp(sensname,"gyro") == 0) {
		if (argc == 2) {
			printf(MSG_CALIBRATION_START, sensname);
			fflush(stdout); // ensure print finishes before calibration pauses the screen
			res = do_gyro_calibration();
		} else if (argc == 5) {
			long samples = strtol(argv[2], nullptr, 0);
			long max_errors = strtol(argv[3], nullptr, 0);
			long timeout = strtol(argv[4], nullptr, 0);
			if (samples < 1 || samples > 1000000 || 
					timeout < 2 || timeout > 10000 ||
					max_errors < 0 || max_errors > 5000) { // sanity checks
				fprintf(stderr, MSG_CALIBRATION_GYRO_WRONG_PARAM);
				return 1;
			}
			printf(MSG_CALIBRATION_START_PARAM,sensname,samples,max_errors,timeout);
			fflush(stdout); // ensure print finishes before calibration pauses the screen
			res = do_gyro_calibration(samples, max_errors, (int) timeout);
		}
		else {
			fprintf(stderr, MSG_CALIBRATION_GYRO_WRONG_PARAM);
			return 1;
		}
		if (res == CALIBRATION_RESULT::SUCCESS) {
			int fd = open(GYRO_DEVICE_PATH, O_RDONLY);
			gyro_scale scales;
			if (ioctl(fd, GYROIOCGSCALE, (unsigned long) &scales) == 0) {
				printf(MSG_CALIBRATION_RESULTS, (double) scales.x_offset, (double) scales.y_offset, (double) scales.z_offset,
						(double) scales.x_scale, (double) scales.y_scale, (double) scales.z_scale);
			}
			close (fd);
		}
	}
	else if (strcmp(sensname,"mag") == 0) {
		long sample_count;
		long max_error_count;
		long total_time;
		long poll_timeout_gap;
		if (argc == 6) { // TODO: Interval check
			sample_count = strtol(argv[2], nullptr, 0);
			max_error_count = strtol(argv[3], nullptr, 0);
			total_time = strtol(argv[4], nullptr, 0);
			poll_timeout_gap = strtol(argv[5], nullptr, 0);
			if (max_error_count < 0 || max_error_count > sample_count ||
					total_time < 1000 || total_time > 1000000 ||
					poll_timeout_gap < 1 || poll_timeout_gap > 100 ||
					sample_count < 100 || (total_time / sample_count) < 5) {
				fprintf(stderr, MSG_CALIBRATION_MAG_WRONG_PARAM);
				return 1;
			}
			// total_time / sample_count should be greater than 10!			
		}
		else if (argc != 2) {
			fprintf(stderr, MSG_CALIBRATION_MAG_WRONG_PARAM);
			return 1;
		}
		printf(MSG_CALIBRATION_START, sensname);
		fflush(stdout); // ensure print finishes before calibration pauses the screen
		res = do_mag_builtin_calibration();
		if (res == CALIBRATION_RESULT::SUCCESS) {
			// Could possibly fail in the future if no "internal calibration" warning will be implemented
			printf(MSG_CALIBRATION_MAG_ROTATE);
			fflush(stdout); // ensure print finishes before calibration pauses the screen
			if (argc== 2) {
				res = do_mag_offset_calibration();
			}
			else {
				res = do_mag_offset_calibration(sample_count, max_error_count, total_time, poll_timeout_gap);
			}
			if (res == CALIBRATION_RESULT::SUCCESS) {
				int fd = open(MAG_DEVICE_PATH, O_RDONLY);
				mag_scale scales;
				if (ioctl(fd, MAGIOCGSCALE, (unsigned long) &scales) == 0) {
					printf(MSG_CALIBRATION_RESULTS, (double) scales.x_offset, (double) scales.y_offset, (double) scales.z_offset,
							(double) scales.x_scale, (double) scales.y_scale, (double) scales.z_scale);
				}
				close (fd);
			}
		}
	}
	else if (strcmp(sensname,"baro") == 0) {
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
	}
	else if (strcmp(sensname,"rc") == 0) {
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
	}
	else if (strcmp(sensname,"airspeed") == 0) {
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
	}
	else if (strcmp(sensname,"all") == 0) {
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
	}
	else {
		fprintf(stderr, MSG_CALIBRATION_WRONG_MODULE, sensname);
		return 1;
	}

	printf(MSG_CALIBRATION_FINISH, res);
	if (res != CALIBRATION_RESULT::SUCCESS && (int)res < errors_size)
	{
		printf(errors[(int)res]); // converts CALIBRATION_RESULT to errors index
		return 1;
	}
	return 0;
}
