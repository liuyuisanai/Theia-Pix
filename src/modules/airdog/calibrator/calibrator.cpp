#include <nuttx/config.h>

#include <drivers/drv_accel.h>
#include <drivers/calibration/calibration.hpp>
#include <drivers/drv_gyro.h> // ioctl commands
#include <drivers/drv_mag.h>
#include <drivers/drv_tone_alarm.h>
#include <systemlib/err.h>
#include <fcntl.h> // open
#include <mavlink/mavlink_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "calibrator.hpp"
#include "calibration_commons.hpp"
#include "accel_calibration.hpp"
#include "gyro_calibration.hpp"
#include "mag_calibration.hpp"

namespace calibration {

enum class TONES : uint8_t {
	PREPARE = 0,
	START,
	WAITING_FOR_USER,
	NEGATIVE,
	FINISHED,
	ERROR,
	STOP // stops the current tune
};

// TODO! Consider using this class for prepare and print_results functions too
enum class SENSOR_TYPE : uint8_t {
	GYRO = 0,
	MAG,
	ACCEL
};

// Common procedure for sensor calibration that waits for the user to get ready
inline void prepare(const char* sensor_type, const int beeper_fd);
// Translates TONES enum to specific tone_alarm tunes
inline void beep(const int beeper_fd, TONES tone);
// Print final calibration results
inline void print_results (CALIBRATION_RESULT res, const char* sensor_type, const int beeper_fd, int mavlink_fd);
// Print calibration scales and offsets currently set
inline void print_scales(SENSOR_TYPE sensor, int mavlink_fd);
// Helper function for print_scales
template <class scale_T> void print_scales_helper(const char* device_path, int command, int mavlink_fd);

__EXPORT bool calibrate_gyroscope(int mavlink_fd, const unsigned int sample_count,
						const unsigned int max_error_count,
						const int timeout) {
	CALIBRATION_RESULT res;
	int beeper_fd = open(TONEALARM_DEVICE_PATH, O_RDONLY);
	if (beeper_fd < 0) { // This is rather critical
		warnx("Gyro calibration could not find beeper device. Aborting.");
		return (false);
	}
	prepare("Gyro", beeper_fd);
	printf("Parameters: samples=%d, error count=%d, timeout=%d\n", sample_count, max_error_count, timeout);
	fflush(stdout);
	res = do_gyro_calibration(sample_count, max_error_count, timeout);
	print_results(res, "Gyro", beeper_fd, mavlink_fd);
	close(beeper_fd);
	if (res == CALIBRATION_RESULT::SUCCESS) {
		print_scales(SENSOR_TYPE::GYRO, mavlink_fd);
		return true;
	}
	else {
		return false;
	}
}

__EXPORT bool calibrate_magnetometer(int mavlink_fd, unsigned int sample_count,
							unsigned int max_error_count,
							unsigned int total_time,
							int poll_timeout_gap) {
	CALIBRATION_RESULT res;
	int beeper_fd = open(TONEALARM_DEVICE_PATH, O_RDONLY);
	if (beeper_fd < 0) { // This is rather critical
		warnx("Mag calibration could not find beeper device. Aborting.");
		return (false);
	}
	prepare("Mag", beeper_fd);
	res = do_mag_builtin_calibration();
	if (res == CALIBRATION_RESULT::SUCCESS) {
		// Could possibly fail in the future if "no internal calibration" warning will be implemented
		beep(beeper_fd, TONES::WAITING_FOR_USER);
		sleep(3); // hack because we don't detect if rotation has started
		beep(beeper_fd, TONES::STOP);
		printf("Sampling magnetometer offsets. Do a full rotation around each axis.\n");
		printf("Parameters: samples=%d, max_errors=%d,\n\ttotal_time=%d ms, timeout_gap=%d ms\n",
				sample_count, max_error_count, total_time, poll_timeout_gap);
		fflush(stdout); // ensure print finishes before calibration pauses the screen
		res = do_mag_offset_calibration(sample_count, max_error_count, total_time, poll_timeout_gap);
	}
	print_results(res, "Mag", beeper_fd, mavlink_fd);
	close(beeper_fd);
	if (res == CALIBRATION_RESULT::SUCCESS) {
		print_scales(SENSOR_TYPE::MAG, mavlink_fd);
		return true;
	}
	else {
		return false;
	}
}

__EXPORT bool calibrate_accelerometer(int mavlink_fd) {
	CALIBRATION_RESULT res;
	const char* axis_labels[] = {
		"+x",
		"-x",
		"+y",
		"-y",
		"+z",
		"-z"
	};
	int beeper_fd = open(TONEALARM_DEVICE_PATH, O_RDONLY);
	if (beeper_fd < 0) { // This is rather critical
		warnx("Accel calibration could not find beeper device. Aborting.");
		return (false);
	}
	prepare("Accel", beeper_fd);

	AccelCalibrator calib;
	res = calib.init();
	if (res == CALIBRATION_RESULT::SUCCESS) {
		while (calib.sampling_needed) {
			printf("Rotate to one of the remaining axes: ");
			for (int i = 0; i < 6; ++i) {
				if (!calib.calibrated_axes[i]) {
					fputs(axis_labels[i], stdout);
					fputs(" ", stdout);
				}
			}
			fputs("\n", stdout);
			fflush(stdout); // ensure puts finished before calibration pauses the screen
			beep(beeper_fd, TONES::WAITING_FOR_USER);
			res = calib.sample_axis();
			if (res == CALIBRATION_RESULT::SUCCESS) {
				beep(beeper_fd, TONES::STOP);
				res = calib.read_samples();
				if (res == CALIBRATION_RESULT::SUCCESS) {
					printf("Successfully sampled the axis.\n");
				}
				else {
					break;
				}
			}
			else if (res == CALIBRATION_RESULT::AXIS_DONE_FAIL) {
				beep(beeper_fd, TONES::STOP);
				sleep(1); // ensures the tunes don't blend too much
				beep(beeper_fd, TONES::NEGATIVE);
				printf("Axis has been sampled already.\n");
				sleep(2); // gives time for negative tune to finish
			}
			else {
				break;
			}
		}
		if (res == CALIBRATION_RESULT::SUCCESS) {
			res = calib.calculate_and_save();
		}
	}

	print_results(res, "Accel", beeper_fd, mavlink_fd);
	close(beeper_fd);
	if (res == CALIBRATION_RESULT::SUCCESS) {
		print_scales(SENSOR_TYPE::ACCEL, mavlink_fd);
		return true;
	}
	else {
		return false;
	}
}

inline void prepare(const char* sensor_type, const int beeper_fd) {
	beep(beeper_fd, TONES::PREPARE);
	printf("%s calibration: preparing... waiting for user.\n", sensor_type);
	sleep(3);
	beep(beeper_fd, TONES::START);
	printf("Starting %s calibration.\n", sensor_type);
	sleep(1); // give some time for the tune to play
}

// TODO we can get rid of this function when tones are finalized.
// Alternatively this function can be used to pass feedback to Mavlink
inline void beep(const int beeper_fd, TONES tone) {
	int mapped_tone;
	switch (tone) {
	case TONES::PREPARE:
		mapped_tone = TONE_NOTIFY_NEUTRAL_TUNE;
		break;
	case TONES::START:
		mapped_tone = TONE_PROCESS_START;
		break;
	case TONES::NEGATIVE:
		mapped_tone = TONE_WRONG_INPUT;
		break;
	case TONES::WAITING_FOR_USER: // should be continuous - tune string starts with MB
		mapped_tone = TONE_WAITING_INPUT;
		break;
	case TONES::FINISHED:
		mapped_tone = TONE_NOTIFY_POSITIVE_TUNE;
		break;
	case TONES::ERROR:
		mapped_tone = TONE_GENERAL_ERROR;
		break;
	case TONES::STOP: // used to stop the "WAITING_FOR_USER" tune
		mapped_tone = TONE_STOP_TUNE;
		break;
	}
	// currently errors are ignored
	ioctl(beeper_fd, TONE_SET_ALARM, mapped_tone);
}

inline void print_results(CALIBRATION_RESULT res, const char* sensor_type, const int beeper_fd, int mavlink_fd) {
	const char* errors[] = { // allows to index by (error code)
		"No errors reported.\n", // code = 0 = SUCCESS
		"Calibration failed.\n", // code = 1 = FAIL
		"Failed to reset sensor scale.\n", // code = 2 = SCALE_RESET_FAIL
		"Failed to apply sensor scale.\n", // code = 3 = SCALE_APPLY_FAIL
		"Failed to get sane data from sensor.\n", // code = 4 = SENSOR_DATA_FAIL
		"Failed to save parameters to EEPROM.\n", // code = 5 = PARAMETER_DEFAULT_FAIL
		"Failed to set scaling parameters.\n", // code = 6 = PARAMETER_SET_FAIL
		"Failed to read sensor scale.\n", // code = 7 = SCALE_READ_FAIL
		"Axis has been sampled already.\n" // code = 8 = AXIS_DONE_FAIL
	};
	const size_t errors_size = sizeof(errors) / sizeof(*errors);

	printf("Calibration finished with status: %d.\n", res);
	if (res != CALIBRATION_RESULT::SUCCESS)	{
		beep(beeper_fd, TONES::ERROR);
		if ((int)res < errors_size) {
			printf(errors[(int)res]); // converts CALIBRATION_RESULT to errors index
			if (mavlink_fd != 0) {
				mavlink_log_critical(mavlink_fd, errors[(int) res]);
			}
		}
	}
	else {
		printf("%s calibration finished successfully.\n", sensor_type);
		if (mavlink_fd != 0) {
			mavlink_log_info(mavlink_fd, "%s calibration finished successfully.\n", sensor_type);
		}
		beep(beeper_fd, TONES::FINISHED);
	}
}

inline void print_scales(SENSOR_TYPE sensor, int mavlink_fd) {
	calibration_values_s calibration;
	int dev_fd;

	switch (sensor) {
	case SENSOR_TYPE::GYRO:
		dev_fd = open(GYRO_DEVICE_PATH, O_RDONLY);
		if (ioctl(dev_fd, GYROIOCGSCALE, (unsigned long) &calibration) == 0) {
			print_calibration(calibration, mavlink_fd);
		}
		break;
	case SENSOR_TYPE::MAG:
		print_scales_helper <mag_scale> (MAG_DEVICE_PATH, MAGIOCGSCALE, mavlink_fd);
		break;
	case SENSOR_TYPE::ACCEL:
		dev_fd = open(ACCEL_DEVICE_PATH, O_RDONLY);
		if (ioctl(dev_fd, ACCELIOCGSCALE, (unsigned long) &calibration) == 0) {
			print_calibration(calibration, mavlink_fd);
		}
		break;
	}
}

template <class scale_T> void print_scales_helper(const char* device_path, int command, int mavlink_fd) {
	scale_T scale;
	int dev_fd = open(device_path, O_RDONLY);
	if (ioctl(dev_fd, command, (unsigned long) &scale) == 0) {
		printf("Result offsets: X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\nResult scales:  X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n",
				(double) scale.x_offset, (double) scale.y_offset, (double) scale.z_offset,
				(double) scale.x_scale, (double) scale.y_scale, (double) scale.z_scale);
		if (mavlink_fd != 0) {
			mavlink_log_info(mavlink_fd, "Result offsets:\n");
			mavlink_log_info(mavlink_fd, "X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n",
				(double) scale.x_offset, (double) scale.y_offset, (double) scale.z_offset);
			mavlink_log_info(mavlink_fd, "Result scales:\n");
			mavlink_log_info(mavlink_fd, "X: % 9.6f, Y: % 9.6f, Z: % 9.6f.\n",
				(double) scale.x_scale, (double) scale.y_scale, (double) scale.z_scale);
		}
	}

}
} // End calibration namespace

// Execution messages
#define MSG_CALIBRATION_USAGE "Usage: %s module_name\nmodule_name is one of accel, gyro, mag, baro, airspeed, rc, all\n" \
			"Advanced mode - gyro supports 3 parameters: sample count, max error count\n" \
			"and timeout in ms (defaults: 5000, 1000, 1000)\n"
#define MSG_CALIBRATION_NOT_IMPLEMENTED "Not supported yet. Sorry.\n"
#define MSG_CALIBRATION_WRONG_MODULE "Unknown module name \"%s\". Try accel, gyro, mag, baro, airspeed, rc, all\n"
#define MSG_CALIBRATION_GYRO_WRONG_PARAM "0 or 3 parameters required.\nValid ranges for samples 1-1000000, for errors 0-5000, for timeout 2-10000.\n"
#define MSG_CALIBRATION_MAG_WRONG_PARAM "0 or 4 parameters required.\nValid ranges for samples 100-total_time/5, for errors 0-sample_count,\nfor time 1-1000000, for gap 1-100.\n"
#define MSG_CALIBRATION_ACCEL_WRONG_PARAM "No parameters supported.\n"

extern "C" __EXPORT int calibrator_main(int argc, char ** argv)
{
	using namespace calibration;
	char* sensname;

	if (argc < 2 || argc > 6) {
		fprintf(stderr, MSG_CALIBRATION_USAGE, argv[0]);
		return 1;
	}
	sensname = argv[1];

	if (strcmp(sensname, "accel") == 0) {
		if (argc != 2) {
			fprintf(stderr, MSG_CALIBRATION_ACCEL_WRONG_PARAM);
			return 1;
		}
		return ((int) !calibrate_accelerometer());
	}
	else if (strcmp(sensname,"gyro") == 0) {
		if (argc == 2) {
			return ((int) calibrate_gyroscope());
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
			return ((int) !calibrate_gyroscope(0, (unsigned int) samples, (unsigned int) max_errors, (int) timeout));
		}
		else {
			fprintf(stderr, MSG_CALIBRATION_GYRO_WRONG_PARAM);
			return 1;
		}
	}
	else if (strcmp(sensname,"mag") == 0) {
		long sample_count;
		long max_error_count;
		long total_time;
		long poll_timeout_gap;
		if (argc == 6) {
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
			return((int) !calibrate_magnetometer(0, (unsigned int) sample_count, (unsigned int) max_error_count, (unsigned int) total_time, (int) poll_timeout_gap));
		}
		else if (argc == 2) {
			return((int) !calibrate_magnetometer());
		}
		else {
			fprintf(stderr, MSG_CALIBRATION_MAG_WRONG_PARAM);
			return 1;
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

	return 0;
}
