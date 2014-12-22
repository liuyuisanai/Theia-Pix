/****************************************************************************
 *
 *   Copyright (c) 2012-2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file sdlog2.c
 *
 * Simple SD logger for flight data. Buffers new sensor values and
 * does the heavy SD I/O in a low-priority worker thread.
 *
 * @author Lorenz Meier <lm@inf.ethz.ch>
 * @author Anton Babushkin <anton.babushkin@me.com>
 */

#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <systemlib/err.h>
#include <unistd.h>
#include <drivers/drv_hrt.h>
#include <math.h>

#include <drivers/drv_range_finder.h>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/sensor_combined.h>
#include <uORB/topics/vehicle_attitude.h>
#include <uORB/topics/vehicle_attitude_setpoint.h>
#include <uORB/topics/vehicle_rates_setpoint.h>
#include <uORB/topics/actuator_outputs.h>
#include <uORB/topics/actuator_controls.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/vehicle_local_position.h>
#include <uORB/topics/vehicle_local_position_setpoint.h>
#include <uORB/topics/vehicle_global_position.h>
#include <uORB/topics/target_global_position.h>
#include <uORB/topics/trajectory.h>
#include <uORB/topics/position_setpoint_triplet.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/vehicle_vicon_position.h>
#include <uORB/topics/vehicle_global_velocity_setpoint.h>
#include <uORB/topics/optical_flow.h>
#include <uORB/topics/battery_status.h>
#include <uORB/topics/differential_pressure.h>
#include <uORB/topics/airspeed.h>
#include <uORB/topics/rc_channels.h>
#include <uORB/topics/esc_status.h>
#include <uORB/topics/telemetry_status.h>
#include <uORB/topics/estimator_status.h>
#include <uORB/topics/system_power.h>
#include <uORB/topics/servorail_status.h>
#include <uORB/topics/airdog_path_log.h>
#include <uORB/topics/mavlink_receive_stats.h>

#include <systemlib/systemlib.h>
#include <systemlib/param/param.h>
#include <version/version.h>

#include <mavlink/mavlink_log.h>

#include "logbuffer.h"
#include <sdlog2/sdlog2_format.h>
#include <sdlog2/sdlog2_messages.h>

#define LOGBUFFER_WRITE_AND_COUNT(_msg) if (logbuffer_write(&lb, &log_msg, LOG_PACKET_SIZE(_msg))) { \
		log_msgs_written++; \
	} else { \
		log_msgs_skipped++; \
	}

#define LOG_ORB_SUBSCRIBE(_var, _topic) subs.##_var##_sub = orb_subscribe(ORB_ID(##_topic##)); \
	fds[fdsc_count].fd = subs.##_var##_sub; \
	fds[fdsc_count].events = POLLIN; \
	fdsc_count++;

static bool main_thread_should_exit = false;		/**< Deamon exit flag */
static bool thread_running = false;			/**< Deamon status flag */
static int deamon_task;						/**< Handle of deamon task / thread */
static bool logwriter_should_exit = false;	/**< Logwriter thread exit flag */
static const int MAX_NO_LOGFOLDER = 999;	/**< Maximum number of log dirs */
static const int MAX_NO_LOGFILE = 999;		/**< Maximum number of log files */
static const int LOG_BUFFER_SIZE_DEFAULT = 8192;
static const int MAX_WRITE_CHUNK = 512;
static const int MIN_BYTES_TO_WRITE = 512;

static const char *log_root = "/fs/microsd/log";
static int mavlink_fd = -1;
struct logbuffer_s lb;

/* mutex / condition to synchronize threads */
static pthread_mutex_t logbuffer_mutex;
static pthread_cond_t logbuffer_cond;

static char log_dir[32];

/* statistics counters */
static uint64_t start_time = 0;
static unsigned long log_bytes_written = 0;
static unsigned long log_msgs_written = 0;
static unsigned long log_msgs_skipped = 0;

/* GPS time, used for log files naming */
static uint64_t gps_time = 0;

/* current state of logging */
static bool logging_enabled = false;
/* use date/time for naming directories and files (-t option) */
static bool log_name_timestamp = false;

/* helper flag to track system state changes */
static bool flag_system_armed = false;

static pthread_t logwriter_pthread = 0;
static pthread_attr_t logwriter_attr;

/**
 * Log buffer writing thread. Open and close file here.
 */
static void *logwriter_thread(void *arg);

/**
 * SD log management function.
 */
__EXPORT int sdlog2_lite_main(int argc, char *argv[]);

static bool copy_if_updated(orb_id_t topic, int handle, void *buffer);

/**
 * Mainloop of sd log deamon.
 */
int sdlog2_lite_thread_main(int argc, char *argv[]);

/**
 * Print the correct usage.
 */
static void sdlog2_lite_usage(const char *reason);

/**
 * Print the current status.
 */
static void sdlog2_lite_status(void);

/**
 * Start logging: create new file and start log writer thread.
 */
static void sdlog2_lite_start_log(void);

/**
 * Stop logging: stop log writer thread and close log file.
 */
static void sdlog2_lite_stop_log(void);

/**
 * Write a header to log file: list of message formats.
 */
static int write_formats(int fd);

/**
 * Write version message to log file.
 */
static int write_version(int fd);

/**
 * Write parameters to log file.
 */
static int write_parameters(int fd);

static bool file_exist(const char *filename);

static int file_copy(const char *file_old, const char *file_new);

static void handle_command(struct airdog_path_log_s *cmd);

/**
 * Create dir for current logging session. Store dir name in 'log_dir'.
 */
static int create_log_dir(void);

/**
 * Select first free log file name and open it.
 */
static int open_log_file(void);

static void
sdlog2_lite_usage(const char *reason)
{
	if (reason) {
		fprintf(stderr, "%s\n", reason);
	}

	errx(1, "usage: sdlog2 {start|stop|status} [-r <log rate>] [-b <buffer size>] -e -a -t\n"
		 "\t-r\tLog rate in Hz, 0 means unlimited rate\n"
		 "\t-b\tLog buffer size in KiB, default is 8\n"
		 "\t-e\tEnable logging by default (if not, can be started by command)\n"
		 "\t-a\tLog only when armed (can be still overriden by command)\n"
		 "\t-t\tUse date/time for naming log directories and files\n");
}

/**
 * The logger deamon app only briefly exists to start
 * the background job. The stack size assigned in the
 * Makefile does only apply to this management task.
 *
 * The actual stack size should be set in the call
 * to task_spawn().
 */
int sdlog2_lite_main(int argc, char *argv[])
{
	if (argc < 2) {
		sdlog2_lite_usage("missing command");
	}

	if (!strcmp(argv[1], "start")) {

		if (thread_running) {
			warnx("already running");
			/* this is not an error */
			exit(0);
		}

		main_thread_should_exit = false;
		deamon_task = task_spawn_cmd("sdlog2",
						 SCHED_DEFAULT,
						 SCHED_PRIORITY_DEFAULT - 30,
						 3000,
						 sdlog2_lite_thread_main,
						 (const char **)argv);
		exit(0);
	}

	if (!strcmp(argv[1], "stop")) {
		if (!thread_running) {
			warnx("not started");
		}

		main_thread_should_exit = true;
		exit(0);
	}

	if (!strcmp(argv[1], "status")) {
		if (thread_running) {
			sdlog2_lite_status();

		} else {
			warnx("not started\n");
		}

		exit(0);
	}

	sdlog2_lite_usage("unrecognized command");
	exit(1);
}

int create_log_dir()
{
	/* create dir on sdcard if needed */
	uint16_t dir_number = 1; // start with dir sess001
	int mkdir_ret;

	if (log_name_timestamp && gps_time != 0) {
		/* use GPS date for log dir naming: e.g. /fs/microsd/2014-01-19 */
		time_t gps_time_sec = gps_time / 1000000;
		struct tm t;
		gmtime_r(&gps_time_sec, &t);
		int n = snprintf(log_dir, sizeof(log_dir), "%s/", log_root);
		strftime(log_dir + n, sizeof(log_dir) - n, "%Y-%m-%d", &t);
		mkdir_ret = mkdir(log_dir, S_IRWXU | S_IRWXG | S_IRWXO);

		if (mkdir_ret == OK) {
			warnx("log dir created: %s", log_dir);

		} else if (errno != EEXIST) {
			warn("failed creating new dir: %s", log_dir);
			return -1;
		}

	} else {
		/* look for the next dir that does not exist */
		while (dir_number <= MAX_NO_LOGFOLDER) {
			/* format log dir: e.g. /fs/microsd/sess001 */
			sprintf(log_dir, "%s/sess%03u", log_root, dir_number);
			mkdir_ret = mkdir(log_dir, S_IRWXU | S_IRWXG | S_IRWXO);

			if (mkdir_ret == 0) {
				warnx("log dir created: %s", log_dir);
				break;

			} else if (errno != EEXIST) {
				warn("failed creating new dir: %s", log_dir);
				return -1;
			}

			/* dir exists already */
			dir_number++;
			continue;
		}

		if (dir_number >= MAX_NO_LOGFOLDER) {
			/* we should not end up here, either we have more than MAX_NO_LOGFOLDER on the SD card, or another problem */
			warnx("all %d possible dirs exist already", MAX_NO_LOGFOLDER);
			return -1;
		}
	}

	/* print logging path, important to find log file later */
	warnx("log dir: %s", log_dir);
	mavlink_log_info(mavlink_fd, "[sdlog2] log dir: %s", log_dir);
	return 0;
}

int open_log_file()
{
	/* string to hold the path to the log */
	char log_file_name[16] = "";
	char log_file_path[48] = "";

	if (log_name_timestamp && gps_time != 0) {
		/* use GPS time for log file naming, e.g. /fs/microsd/2014-01-19/19_37_52.bin */
		time_t gps_time_sec = gps_time / 1000000;
		struct tm t;
		gmtime_r(&gps_time_sec, &t);
		strftime(log_file_name, sizeof(log_file_name), "%H_%M_%S.bin", &t);
		snprintf(log_file_path, sizeof(log_file_path), "%s/%s", log_dir, log_file_name);

	} else {
		uint16_t file_number = 1; // start with file log001

		/* look for the next file that does not exist */
		while (file_number <= MAX_NO_LOGFILE) {
			/* format log file path: e.g. /fs/microsd/sess001/log001.bin */
			snprintf(log_file_name, sizeof(log_file_name), "log%03u.bin", file_number);
			snprintf(log_file_path, sizeof(log_file_path), "%s/%s", log_dir, log_file_name);

			if (!file_exist(log_file_path)) {
				break;
			}

			file_number++;
		}

		if (file_number > MAX_NO_LOGFILE) {
			/* we should not end up here, either we have more than MAX_NO_LOGFILE on the SD card, or another problem */
			warnx("all %d possible files exist already", MAX_NO_LOGFILE);
			return -1;
		}
	}

	int fd = open(log_file_path, O_CREAT | O_WRONLY | O_DSYNC);

	if (fd < 0) {
		warn("failed opening log: %s", log_file_name);
		mavlink_log_info(mavlink_fd, "[sdlog2] failed opening log: %s", log_file_name);

	} else {
		warnx("log file: %s", log_file_name);
		mavlink_log_info(mavlink_fd, "[sdlog2] log file: %s", log_file_name);
	}

	return fd;
}

static void *logwriter_thread(void *arg)
{
	/* set name */
	prctl(PR_SET_NAME, "sdlog2_lite_writer", 0);

	int log_fd = open_log_file();

	if (log_fd < 0) {
		return NULL;
	}

	struct logbuffer_s *logbuf = (struct logbuffer_s *)arg;

	/* write log messages formats, version and parameters */
	log_bytes_written += write_formats(log_fd);

	log_bytes_written += write_version(log_fd);

	log_bytes_written += write_parameters(log_fd);

	fsync(log_fd);

	int poll_count = 0;

	void *read_ptr;

	int n = 0;

	bool should_wait = false;

	bool is_part = false;

	while (true) {
		/* make sure threads are synchronized */
		pthread_mutex_lock(&logbuffer_mutex);

		/* update read pointer if needed */
		if (n > 0) {
			logbuffer_mark_read(&lb, n);
		}

		/* only wait if no data is available to process */
		if (should_wait && !logwriter_should_exit) {
			/* blocking wait for new data at this line */
			pthread_cond_wait(&logbuffer_cond, &logbuffer_mutex);
		}

		/* only get pointer to thread-safe data, do heavy I/O a few lines down */
		int available = logbuffer_get_ptr(logbuf, &read_ptr, &is_part);

		/* continue */
		pthread_mutex_unlock(&logbuffer_mutex);

		if (available > 0) {
			/* do heavy IO here */
			if (available > MAX_WRITE_CHUNK) {
				n = MAX_WRITE_CHUNK;

			} else {
				n = available;
			}

			n = write(log_fd, read_ptr, n);

			should_wait = (n == available) && !is_part;

			if (n < 0) {
				main_thread_should_exit = true;
				err(1, "error writing log file");
			}

			if (n > 0) {
				log_bytes_written += n;
			}

		} else {
			n = 0;

			/* exit only with empty buffer */
			if (main_thread_should_exit || logwriter_should_exit) {
				break;
			}

			should_wait = true;
		}

		if (++poll_count == 10) {
			fsync(log_fd);
			poll_count = 0;
		}
	}

	fsync(log_fd);
	close(log_fd);

	return NULL;
}

void sdlog2_lite_start_log()
{
	warnx("start logging");
	mavlink_log_info(mavlink_fd, "[sdlog2] start logging");

	/* create log dir if needed */
	if (create_log_dir() != 0) {
		mavlink_log_critical(mavlink_fd, "[sdlog2] error creating log dir");
		errx(1, "error creating log dir");
	}

	/* initialize statistics counter */
	log_bytes_written = 0;
	start_time = hrt_absolute_time();
	log_msgs_written = 0;
	log_msgs_skipped = 0;

	/* initialize log buffer emptying thread */
	pthread_attr_init(&logwriter_attr);

	struct sched_param param;
	/* low priority, as this is expensive disk I/O */
	param.sched_priority = SCHED_PRIORITY_DEFAULT - 40;
	(void)pthread_attr_setschedparam(&logwriter_attr, &param);

	pthread_attr_setstacksize(&logwriter_attr, 2048);

	logwriter_should_exit = false;

	/* start log buffer emptying thread */
	if (0 != pthread_create(&logwriter_pthread, &logwriter_attr, logwriter_thread, &lb)) {
		errx(1, "error creating logwriter thread");
	}

	logging_enabled = true;
}

void sdlog2_lite_stop_log()
{
	warnx("stop logging");
	mavlink_log_info(mavlink_fd, "[sdlog2] stop logging");

	logging_enabled = false;

	/* wake up write thread one last time */
	pthread_mutex_lock(&logbuffer_mutex);
	logwriter_should_exit = true;
	pthread_cond_signal(&logbuffer_cond);
	/* unlock, now the writer thread may return */
	pthread_mutex_unlock(&logbuffer_mutex);

	/* wait for write thread to return */
	int ret;

	if ((ret = pthread_join(logwriter_pthread, NULL)) != 0) {
		warnx("error joining logwriter thread: %i", ret);
	}

	logwriter_pthread = 0;
	pthread_attr_destroy(&logwriter_attr);

	sdlog2_lite_status();
}

int write_formats(int fd)
{
	/* construct message format packet */
	struct {
		LOG_PACKET_HEADER;
		struct log_format_s body;
	} log_msg_format = {
		LOG_PACKET_HEADER_INIT(LOG_FORMAT_MSG),
	};

	int written = 0;

	/* fill message format packet for each format and write it */
	for (int i = 0; i < log_formats_num; i++) {
		log_msg_format.body = log_formats[i];
		written += write(fd, &log_msg_format, sizeof(log_msg_format));
	}

	return written;
}

int write_version(int fd)
{
	/* construct version message */
	struct {
		LOG_PACKET_HEADER;
		struct log_VER_s body;
	} log_msg_VER = {
		LOG_PACKET_HEADER_INIT(LOG_VER_MSG),
	};

	/* fill version message and write it */
	strncpy(log_msg_VER.body.fw_git, FW_GIT, sizeof(log_msg_VER.body.fw_git));
	strncpy(log_msg_VER.body.arch, HW_ARCH, sizeof(log_msg_VER.body.arch));
	return write(fd, &log_msg_VER, sizeof(log_msg_VER));
}

int write_parameters(int fd)
{
	/* construct parameter message */
	struct {
		LOG_PACKET_HEADER;
		struct log_PARM_s body;
	} log_msg_PARM = {
		LOG_PACKET_HEADER_INIT(LOG_PARM_MSG),
	};

	int written = 0;
   param_t params_cnt = param_count();

	for (param_t param = 0; param < params_cnt; param++) {
		/* fill parameter message and write it */
		strncpy(log_msg_PARM.body.name, param_name(param), sizeof(log_msg_PARM.body.name));
		float value = NAN;

		switch (param_type(param)) {
		case PARAM_TYPE_INT32: {
				int32_t i;
				param_get(param, &i);
				value = i;	// cast integer to float
				break;
			}

		case PARAM_TYPE_FLOAT:
			param_get(param, &value);
			break;

		default:
			break;
		}

		log_msg_PARM.body.value = value;
		written += write(fd, &log_msg_PARM, sizeof(log_msg_PARM));
	}

	return written;
}

bool copy_if_updated(orb_id_t topic, int handle, void *buffer)
{
	bool updated;

	orb_check(handle, &updated);

	if (updated) {
		orb_copy(topic, handle, buffer);
	}

	return updated;
}

int sdlog2_lite_thread_main(int argc, char *argv[])
{
	mavlink_fd = open(MAVLINK_LOG_DEVICE, 0);

	if (mavlink_fd < 0) {
		warnx("failed to open MAVLink log stream, start mavlink app first");
	}

	/* delay = 1 / rate (rate defined by -r option), default log rate: 50 Hz */
	useconds_t sleep_delay = 20000;
	int log_buffer_size = LOG_BUFFER_SIZE_DEFAULT;
	logging_enabled = false;
	/* enable logging on start (-e option) */
	bool log_on_start = false;
	/* enable logging when armed (-a option) */
	// bool log_when_armed = false;
	log_name_timestamp = false;

	flag_system_armed = false;

	/* work around some stupidity in task_create's argv handling */
	argc -= 2;
	argv += 2;
	int ch;

	/* don't exit from getopt loop to leave getopt global variables in consistent state,
	 * set error flag instead */
	bool err_flag = false;
    
    while ((ch = getopt(argc, argv, "r:b:et")) != EOF) {
		switch (ch) {
		case 'r': {
				unsigned long r = strtoul(optarg, NULL, 10);

				if (r <= 0) {
					r = 1;
				}

				sleep_delay = 1000000 / r;
			}
			break;

		case 'b': {
				unsigned long s = strtoul(optarg, NULL, 10);

				if (s < 1) {
					s = 1;
				}

				log_buffer_size = 1024 * s;
			}
			break;

		case 'e':
			log_on_start = true;
			break;

		/* case 'a':
			log_when_armed = true;
			break; */

		case 't':
			log_name_timestamp = true;
			break;

		case '?':
			if (optopt == 'c') {
				warnx("option -%c requires an argument", optopt);

			} else if (isprint(optopt)) {
				warnx("unknown option `-%c'", optopt);

			} else {
				warnx("unknown option character `\\x%x'", optopt);
			}
			err_flag = true;
			break;

		default:
			warnx("unrecognized flag");
			err_flag = true;
			break;
		}
	}

	if (err_flag) {
		sdlog2_lite_usage(NULL);
	}

	gps_time = 0;

	/* create log root dir */
	int mkdir_ret = mkdir(log_root, S_IRWXU | S_IRWXG | S_IRWXO);

	if (mkdir_ret != 0 && errno != EEXIST) {
		err(1, "failed creating log root dir: %s", log_root);
	}

	/* copy conversion scripts */
	const char *converter_in = "/etc/logging/conv.zip";
	char *converter_out = malloc(64);
	snprintf(converter_out, 64, "%s/conv.zip", log_root);

	if (file_copy(converter_in, converter_out) != OK) {
		warn("unable to copy conversion scripts");
	}

	free(converter_out);

	/* initialize log buffer with specified size */
	warnx("log buffer size: %i bytes", log_buffer_size);

	if (OK != logbuffer_init(&lb, log_buffer_size)) {
		errx(1, "can't allocate log buffer, exiting");
	}

	struct vehicle_status_s buf_status;

	struct vehicle_gps_position_s buf_gps_pos;

	memset(&buf_status, 0, sizeof(buf_status));

	memset(&buf_gps_pos, 0, sizeof(buf_gps_pos));

	/* warning! using union here to save memory, elements should be used separately! */
	union {
		struct airdog_path_log_s cmd;
		struct sensor_combined_s sensor;
		struct vehicle_attitude_s att;
		struct vehicle_attitude_setpoint_s att_sp;
		struct vehicle_rates_setpoint_s rates_sp;
		struct actuator_outputs_s act_outputs;
		struct actuator_controls_s act_controls;
		struct vehicle_local_position_s local_pos;
		struct vehicle_local_position_setpoint_s local_pos_sp;
		struct vehicle_global_position_s global_pos;
		struct position_setpoint_triplet_s triplet;
		struct vehicle_vicon_position_s vicon_pos;
		struct optical_flow_s flow;
		struct rc_channels_s rc;
		struct differential_pressure_s diff_pres;
		struct airspeed_s airspeed;
		struct esc_status_s esc;
		struct vehicle_global_velocity_setpoint_s global_vel_sp;
		struct battery_status_s battery;
		struct telemetry_status_s telemetry;
		struct range_finder_report range_finder;
		struct estimator_status_report estimator_status;
		struct system_power_s system_power;
		struct servorail_status_s servorail_status;
		struct target_global_position_s target_pos;
		struct trajectory_s trajectory;
		struct mavlink_receive_stats_s mav_stats;
	} buf;

	memset(&buf, 0, sizeof(buf));

	/* log message buffer: header + body */
#pragma pack(push, 1)
	struct {
		LOG_PACKET_HEADER;
		union {
			struct log_TIME_s log_TIME;
			struct log_GPS_s log_GPS;
			struct log_STAT_s log_STAT;
			struct log_GPOS_s log_GPOS;
			struct log_LPOS_s log_LPOS;
			struct log_LOTJ_s log_LOTJ;
			struct log_MVST_s log_MVST;
		} body;
	} log_msg = {
		LOG_PACKET_HEADER_INIT(0)
	};
#pragma pack(pop)
	memset(&log_msg.body, 0, sizeof(log_msg.body));

	struct {
		int cmd_sub;
		int status_sub;
		int sensor_sub;
		int att_sub;
		int att_sp_sub;
		int rates_sp_sub;
		int act_outputs_sub;
		int act_controls_sub;
		int local_pos_sub;
		int local_pos_sp_sub;
		int global_pos_sub;
		int triplet_sub;
		int gps_pos_sub;
		int vicon_pos_sub;
		int flow_sub;
		int rc_sub;
		int airspeed_sub;
		int esc_sub;
		int global_vel_sp_sub;
		int battery_sub;
		int telemetry_subs[TELEMETRY_STATUS_ORB_ID_NUM];
		int range_finder_sub;
		int estimator_status_sub;
		int system_power_sub;
		int servorail_status_sub;
		int target_pos_sub;
		int trajectory_sub;
		int mav_stats_sub;
	} subs;

	subs.cmd_sub = orb_subscribe(ORB_ID(airdog_path_log));
	subs.status_sub = orb_subscribe(ORB_ID(vehicle_status));
	subs.gps_pos_sub = orb_subscribe(ORB_ID(vehicle_gps_position));
	subs.sensor_sub = orb_subscribe(ORB_ID(sensor_combined));
	subs.att_sub = orb_subscribe(ORB_ID(vehicle_attitude));
	subs.att_sp_sub = orb_subscribe(ORB_ID(vehicle_attitude_setpoint));
	subs.rates_sp_sub = orb_subscribe(ORB_ID(vehicle_rates_setpoint));
	subs.act_outputs_sub = orb_subscribe(ORB_ID_VEHICLE_CONTROLS);
	subs.act_controls_sub = orb_subscribe(ORB_ID_VEHICLE_ATTITUDE_CONTROLS);
	subs.local_pos_sub = orb_subscribe(ORB_ID(vehicle_local_position));
	subs.local_pos_sp_sub = orb_subscribe(ORB_ID(vehicle_local_position_setpoint));
	subs.global_pos_sub = orb_subscribe(ORB_ID(vehicle_global_position));
	subs.triplet_sub = orb_subscribe(ORB_ID(position_setpoint_triplet));
	subs.vicon_pos_sub = orb_subscribe(ORB_ID(vehicle_vicon_position));
	subs.flow_sub = orb_subscribe(ORB_ID(optical_flow));
	subs.rc_sub = orb_subscribe(ORB_ID(rc_channels));
	subs.airspeed_sub = orb_subscribe(ORB_ID(airspeed));
	subs.esc_sub = orb_subscribe(ORB_ID(esc_status));
	subs.global_vel_sp_sub = orb_subscribe(ORB_ID(vehicle_global_velocity_setpoint));
	subs.battery_sub = orb_subscribe(ORB_ID(battery_status));
	for (int i = 0; i < TELEMETRY_STATUS_ORB_ID_NUM; i++) {
		subs.telemetry_subs[i] = orb_subscribe(telemetry_status_orb_id[i]);
	}
	subs.range_finder_sub = orb_subscribe(ORB_ID(sensor_range_finder));
	subs.estimator_status_sub = orb_subscribe(ORB_ID(estimator_status));
	subs.system_power_sub = orb_subscribe(ORB_ID(system_power));
	subs.servorail_status_sub = orb_subscribe(ORB_ID(servorail_status));
	subs.target_pos_sub = orb_subscribe(ORB_ID(target_global_position));
	subs.trajectory_sub = orb_subscribe(ORB_ID(trajectory));
	subs.mav_stats_sub = orb_subscribe(ORB_ID(mavlink_receive_stats));


	thread_running = true;

	/* initialize thread synchronization */
	pthread_mutex_init(&logbuffer_mutex, NULL);
	pthread_cond_init(&logbuffer_cond, NULL);

	/* enable logging on start if needed */
	if (log_on_start) {
		/* check GPS topic to get GPS time */
		if (log_name_timestamp) {
			if (copy_if_updated(ORB_ID(vehicle_gps_position), subs.gps_pos_sub, &buf_gps_pos)) {
				gps_time = buf_gps_pos.time_gps_usec;
			}
		}

		sdlog2_lite_start_log();
	}

	while (!main_thread_should_exit) {
		usleep(sleep_delay);
		/* --- VEHICLE COMMAND - LOG MANAGEMENT --- */
		if (copy_if_updated(ORB_ID(airdog_path_log), subs.cmd_sub, &buf.cmd)) {
			handle_command(&buf.cmd);
		}

		/* --- GPS POSITION - LOG MANAGEMENT --- */
		bool gps_pos_updated = copy_if_updated(ORB_ID(vehicle_gps_position), subs.gps_pos_sub, &buf_gps_pos);

		if (gps_pos_updated && log_name_timestamp) {
			gps_time = buf_gps_pos.time_gps_usec;
		}

        bool status_updated = copy_if_updated(ORB_ID(vehicle_status), subs.status_sub, &buf_status);

		if (!logging_enabled) {
			continue;
		}

		pthread_mutex_lock(&logbuffer_mutex);

		/* write time stamp message */
		log_msg.msg_type = LOG_TIME_MSG;
		log_msg.body.log_TIME.t = hrt_absolute_time();
		LOGBUFFER_WRITE_AND_COUNT(TIME);

		/* --- VEHICLE STATUS --- */
		if (status_updated) {
			log_msg.msg_type = LOG_STAT_MSG;
			log_msg.body.log_STAT.main_state = (uint8_t) buf_status.main_state;
			log_msg.body.log_STAT.arming_state = (uint8_t) buf_status.arming_state;
			log_msg.body.log_STAT.failsafe_state = (uint8_t) buf_status.failsafe;
			log_msg.body.log_STAT.battery_remaining = buf_status.battery_remaining;
			log_msg.body.log_STAT.battery_warning = (uint8_t) buf_status.battery_warning;
			log_msg.body.log_STAT.landed = (uint8_t) buf_status.condition_landed;
			LOGBUFFER_WRITE_AND_COUNT(STAT);
		}

		/* --- GPS POSITION - UNIT #1 --- */
		if (gps_pos_updated) {

			log_msg.msg_type = LOG_GPS_MSG;
			log_msg.body.log_GPS.gps_time = buf_gps_pos.time_gps_usec;
			log_msg.body.log_GPS.fix_type = buf_gps_pos.fix_type;
			log_msg.body.log_GPS.eph = buf_gps_pos.eph;
			log_msg.body.log_GPS.epv = buf_gps_pos.epv;
			log_msg.body.log_GPS.lat = buf_gps_pos.lat;
			log_msg.body.log_GPS.lon = buf_gps_pos.lon;
			log_msg.body.log_GPS.alt = buf_gps_pos.alt * 0.001f;
			log_msg.body.log_GPS.vel_n = buf_gps_pos.vel_n_m_s;
			log_msg.body.log_GPS.vel_e = buf_gps_pos.vel_e_m_s;
			log_msg.body.log_GPS.vel_d = buf_gps_pos.vel_d_m_s;
			log_msg.body.log_GPS.cog = buf_gps_pos.cog_rad;
			log_msg.body.log_GPS.sats = buf_gps_pos.satellites_used;
			log_msg.body.log_GPS.snr_mean = 0;
			log_msg.body.log_GPS.noise_per_ms = buf_gps_pos.noise_per_ms;
			log_msg.body.log_GPS.jamming_indicator = buf_gps_pos.jamming_indicator;
			LOGBUFFER_WRITE_AND_COUNT(GPS);
		}

		/* --- GLOBAL POSITION --- */
		if (copy_if_updated(ORB_ID(vehicle_global_position), subs.global_pos_sub, &buf.global_pos)) {
			log_msg.msg_type = LOG_GPOS_MSG;
			log_msg.body.log_GPOS.lat = buf.global_pos.lat * 1e7;
			log_msg.body.log_GPOS.lon = buf.global_pos.lon * 1e7;
			log_msg.body.log_GPOS.alt = buf.global_pos.alt;
			log_msg.body.log_GPOS.vel_n = buf.global_pos.vel_n;
			log_msg.body.log_GPOS.vel_e = buf.global_pos.vel_e;
			log_msg.body.log_GPOS.vel_d = buf.global_pos.vel_d;
			log_msg.body.log_GPOS.eph = buf.global_pos.eph;
			log_msg.body.log_GPOS.epv = buf.global_pos.epv;
			LOGBUFFER_WRITE_AND_COUNT(GPOS);
		}

		/* --- LOCAL POSITION --- */
		if (copy_if_updated(ORB_ID(vehicle_local_position), subs.local_pos_sub, &buf.local_pos)) {
			log_msg.msg_type = LOG_LPOS_MSG;
			log_msg.body.log_LPOS.x = buf.local_pos.x;
			log_msg.body.log_LPOS.y = buf.local_pos.y;
			log_msg.body.log_LPOS.z = buf.local_pos.z;
			log_msg.body.log_LPOS.ground_dist = buf.local_pos.dist_bottom;
			log_msg.body.log_LPOS.ground_dist_rate = buf.local_pos.dist_bottom_rate;
			log_msg.body.log_LPOS.vx = buf.local_pos.vx;
			log_msg.body.log_LPOS.vy = buf.local_pos.vy;
			log_msg.body.log_LPOS.vz = buf.local_pos.vz;
			log_msg.body.log_LPOS.ref_lat = buf.local_pos.ref_lat * 1e7;
			log_msg.body.log_LPOS.ref_lon = buf.local_pos.ref_lon * 1e7;
			log_msg.body.log_LPOS.ref_alt = buf.local_pos.ref_alt;
			log_msg.body.log_LPOS.pos_flags = (buf.local_pos.xy_valid ? 1 : 0) |
											  (buf.local_pos.z_valid ? 2 : 0) |
											  (buf.local_pos.v_xy_valid ? 4 : 0) |
											  (buf.local_pos.v_z_valid ? 8 : 0) |
											  (buf.local_pos.xy_global ? 16 : 0) |
											  (buf.local_pos.z_global ? 32 : 0);
			log_msg.body.log_LPOS.landed = buf.local_pos.landed;
			log_msg.body.log_LPOS.ground_dist_flags = (buf.local_pos.dist_bottom_valid ? 1 : 0);
			log_msg.body.log_LPOS.eph = buf.local_pos.eph;
			log_msg.body.log_LPOS.epv = buf.local_pos.epv;
			LOGBUFFER_WRITE_AND_COUNT(LPOS);
		}
		if (copy_if_updated(ORB_ID(trajectory), subs.trajectory_sub, &buf.trajectory)) {
			log_msg.msg_type = LOG_LOTJ_MSG;
			log_msg.body.log_LOTJ.point_type = buf.trajectory.point_type;
			log_msg.body.log_LOTJ.timestamp = buf.trajectory.timestamp;
			log_msg.body.log_LOTJ.lat = buf.trajectory.lat * 1e7d;
			log_msg.body.log_LOTJ.lon = buf.trajectory.lon * 1e7d;
			log_msg.body.log_LOTJ.alt = buf.trajectory.alt;
			log_msg.body.log_LOTJ.relative_alt = buf.trajectory.relative_alt;
			log_msg.body.log_LOTJ.vel_n = buf.trajectory.vel_n;
			log_msg.body.log_LOTJ.vel_e = buf.trajectory.vel_e;
			log_msg.body.log_LOTJ.vel_d = buf.trajectory.vel_d;
			log_msg.body.log_LOTJ.heading = buf.trajectory.heading;
			LOGBUFFER_WRITE_AND_COUNT(LOTJ);
		}

		if (copy_if_updated(ORB_ID(mavlink_receive_stats), subs.mav_stats_sub, &buf.mav_stats)) {
			log_msg.msg_type = LOG_MVST_MSG;
			log_msg.body.log_MVST.total_bytes = buf.mav_stats.total_bytes;
			log_msg.body.log_MVST.gpos_count = buf.mav_stats.gpos_count;
			log_msg.body.log_MVST.heartbeat_count = buf.mav_stats.heartbeat_count;
			log_msg.body.log_MVST.trajectory_count = buf.mav_stats.trajectory_count;

			LOGBUFFER_WRITE_AND_COUNT(MVST);
		}

		/* signal the other thread new data, but not yet unlock */
		if (logbuffer_count(&lb) > MIN_BYTES_TO_WRITE) {
			/* only request write if several packets can be written at once */
			pthread_cond_signal(&logbuffer_cond);
		}

		/* unlock, now the writer thread may run */
		pthread_mutex_unlock(&logbuffer_mutex);
	}

	if (logging_enabled) {
		sdlog2_lite_stop_log();
	}

	pthread_mutex_destroy(&logbuffer_mutex);
	pthread_cond_destroy(&logbuffer_cond);

	free(lb.data);

	warnx("exiting");

	thread_running = false;

	return 0;
}

void sdlog2_lite_status()
{
	float kibibytes = log_bytes_written / 1024.0f;
	float mebibytes = kibibytes / 1024.0f;
	float seconds = ((float)(hrt_absolute_time() - start_time)) / 1000000.0f;

	warnx("wrote %lu msgs, %4.2f MiB (average %5.3f KiB/s), skipped %lu msgs", log_msgs_written, (double)mebibytes, (double)(kibibytes / seconds), log_msgs_skipped);
	mavlink_log_info(mavlink_fd, "[sdlog2] wrote %lu msgs, skipped %lu msgs", log_msgs_written, log_msgs_skipped);
}

/**
 * @return 0 if file exists
 */
bool file_exist(const char *filename)
{
	struct stat buffer;
	return stat(filename, &buffer) == 0;
}

int file_copy(const char *file_old, const char *file_new)
{
	FILE *source, *target;
	source = fopen(file_old, "r");
	int ret = 0;

	if (source == NULL) {
		warnx("failed opening input file to copy");
		return 1;
	}

	target = fopen(file_new, "w");

	if (target == NULL) {
		fclose(source);
		warnx("failed to open output file to copy");
		return 1;
	}

	char buf[128];
	int nread;

	while ((nread = fread(buf, 1, sizeof(buf), source)) > 0) {
		ret = fwrite(buf, 1, nread, target);

		if (ret <= 0) {
			warnx("error writing file");
			ret = 1;
			break;
		}
	}

	fsync(fileno(target));

	fclose(source);
	fclose(target);

	return OK;
}

void handle_command(struct airdog_path_log_s *cmd)
{

    /* request to set different system mode */
    if (cmd->start == 1 && cmd->stop == 0) {
        sdlog2_lite_start_log();
    } else if (cmd->start == 0 && cmd->stop == 1) {
        sdlog2_lite_stop_log();
    }
}
