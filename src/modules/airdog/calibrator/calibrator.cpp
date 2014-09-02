#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <drivers/drv_gyro.h> // ioctl commands
#include <fcntl.h> // open

#include "mag_calibration.hpp"
#include "gyro_calibration.hpp"
#include "calibration_commons.hpp"

// Execution messages
#define MSG_CALIBRATION_START "Starting %s calibration. Hold the system still.\n"
#define MSG_CALIBRATION_FINISH "Calibration finished with status: %d.\n"
#define MSG_CALIBRATION_USAGE "Usage: %s module_name\nmodule_name is one of accel, gyro, mag, baro, airspeed, rc, all\n" \
			"Advanced mode - gyro supports 3 parameters: sample count, max error count and timeout in ms (defaults: 5000, 1000, 1000)\n"
#define MSG_CALIBRATION_NOT_IMPLEMENTED "Not supported yet. Sorry.\n"
#define MSG_CALIBRATION_WRONG_MODULE "Unknown module name \"%s\". Try accel, gyro, mag, baro, airspeed, rc, all\n"
#define MSG_CALIBRATION_RESULTS	"Result offsets: X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\nResult scales:  X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n"
#define MSG_CALIBRATION_WRONG_PARAM "0 or 3 parameters required. Valid values for samples 1-1000000, for errors 0-5000, for timeout 2-10000.\n"
#define MSG_CALIBRATION_START_PARAM	"Starting %s calibration with parameters: samples = %d, max errors = %d, timeout = %d.\n"

using namespace calibration;

extern "C" __EXPORT int calibrator_main(int argc, char ** argv)
{
	char* sensname;
	CALIBRATION_RESULT res = CALIBRATION_RESULT::SUCCESS;

	const char* errors[] = { // allows to index by (error code)
		"No errors reported.\n", // code = 0 = SUCCESS
		"Calibration failed.\n", // code = 1 = FAIL
		"Failed to reset sensor scale.\n", // code = 2 = SCALE_RESET_FAIL
		"Failed to apply sensor scale.\n", // code = 3 = SCALE_APPLY_FAIL
		"Failed to get sane data from sensor.\n", // code = 4 = SENSOR_DATA_FAIL
		"Failed to save parameters to EEPROM.\n", // code = 5 = PARAMETER_DEFAULT_FAIL
		"Failed to set scaling parameters.\n" // code = 6 = PARAMETER_SET_FAIL
	};
	const size_t errors_size = sizeof(errors) / sizeof(*errors);

	if (argc < 2 || argc > 5) {
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
			fflush(stdout);
			res = do_gyro_calibration();
		} else if (argc == 5) {
			long samples = strtol(argv[2], nullptr, 0);
			if (samples < 1 || samples > 1000000) { // sanity check
				fprintf(stderr, MSG_CALIBRATION_WRONG_PARAM);
				return 1;
			}
			long max_errors = strtol(argv[3], nullptr, 0);
			if (max_errors < 0 || max_errors > 5000) { // sanity check
				fprintf(stderr, MSG_CALIBRATION_WRONG_PARAM);
				return 1;
			}
			long timeout = strtol(argv[4], nullptr, 0);
			if (timeout < 2 || timeout > 10000) { // sanity check
				fprintf(stderr, MSG_CALIBRATION_WRONG_PARAM);
				return 1;
			}
			printf(MSG_CALIBRATION_START_PARAM,sensname,samples,max_errors,timeout);
			fflush(stdout); // ensure print finishes before calibration pauses the screen
			res = do_gyro_calibration(samples, max_errors, timeout);
		}
		else {
			fprintf(stderr, MSG_CALIBRATION_WRONG_PARAM);
			return 1;
		}
		printf(MSG_CALIBRATION_FINISH, res);
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
		/* printf(messages[0], sensname);
		fflush(stdout);
		res = do_mag_calibration();
		printf(messages[1], res);*/
		fprintf(stderr, MSG_CALIBRATION_NOT_IMPLEMENTED);
		return 1;
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


	if (res != CALIBRATION_RESULT::SUCCESS && (int)res < errors_size)
	{
		printf(errors[(int)res]); // converts CALIBRATION_RESULT to errors index
		return 1;
	}
	return 0;
}
