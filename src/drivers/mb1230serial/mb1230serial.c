/**
 * The deamon app only briefly exists to start
 * the background job. The stack size assigned in the
 * Makefile does only apply to this management task.
 * 
 * The actual stack size should be set in the call
 * to task_create().
 * http://nuttx.org/Documentation/NuttxUserGuide.html#taskcreate
 */
#include <nuttx/config.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <float.h>
#include <nuttx/sched.h>
#include <sys/prctl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <uORB/uORB.h>
#include <drivers/drv_hrt.h>
#include <drivers/drv_range_finder.h>
#include <systemlib/perf_counter.h>
#include <systemlib/systemlib.h>
#include <systemlib/err.h>
#include <poll.h>

/**
 * Minimal and maximal distances are get from
 * http://www.maxbotix.com/documents/XL-MaxSonar-EZ_Datasheet.pdf
 * Measure unit: METER
 */
__EXPORT const float MINIMAL_DISTANCE = 0.25f; // It realy is 0.2 but due to lags on land increased
__EXPORT const float MAXIMAL_DISTANCE = 7.0f;

/**
 * Experimentally found scale value
 */
#define DISTANCE_SCALE 0.01f


__EXPORT int mb1230serial_main(int argc, char *argv[]);
int mb1230serial_thread_main(int argc, char *argv[]);

static bool thread_should_run = true;	 /**< Daemon running flag */
static bool thread_running = false;	 /**< Daemon status flag */
static bool test_sonar = false; /**< Output to console flag */
static int daemon_task;			 /**< Handle of daemon task / thread */

/**
 * Print the correct usage.
 */
void
usage(const char *reason)
{
	if (reason)
		fprintf(stderr, "%s\n", reason);
	fprintf(stderr, "usage: mb1230serial {start device_path | stop | info}\n\n");
	exit(1);
}

int mb1230serial_main(int argc, char *argv[])
{
	if (argc < 1)
		usage("missing command");

	if (!strcmp(argv[1], "start"))
	{
		if (thread_running)
		{
			warnx("already running\n");
			/* this is not an error */
			exit(0);
		}

		if (argc < 3)
		{
			usage("missing device path");
		}

		thread_should_run = true;
		daemon_task = task_spawn_cmd("mb1230serial",
					 SCHED_DEFAULT,
					 SCHED_PRIORITY_MAX - 5,
					 2000,
					 mb1230serial_thread_main,
					 (argv) ? (const char **)&argv[2] : (const char **)NULL);
		exit(0);
	}

	if (!strcmp(argv[1], "stop"))
	{
		thread_should_run = false;
		exit(0);
	}

    if (!strcmp(argv[1], "info"))
    {
        fprintf(stdout, "mb1230serial_main status: %s\n", thread_running ? "running" : "not running");
        exit(0);
    }

    if (!strcmp(argv[1], "test"))
    {
        if (!thread_running) {
            usage("thread not running yet");
            exit(1);
        }
        else {
            test_sonar = true;
            exit(0);
        }
    }

	usage("unrecognized command");
	exit(1);
}

void cfsetraw(struct termios *termios_s)
{
	termios_s->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
							  | INLCR | IGNCR | ICRNL | IXON);
	termios_s->c_oflag &= ~OPOST;
	termios_s->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios_s->c_cflag &= ~(CSIZE | PARENB | CSTOPB);
	termios_s->c_cflag |= CS8 | CLOCAL;
}

void setup_serial(int fd)
{
	struct termios options;
	int termios_state;

	if ((termios_state = tcgetattr(fd, &options)) < 0)
	{
		close(fd);
		err(-1, "ERR GET CONF: %d\n", termios_state);
	}

	cfsetraw(&options);
	cfsetspeed(&options, B9600);

	if ((termios_state = tcsetattr(fd, TCSANOW, &options)) < 0)
	{
		close(fd);
		err(-1, "ERR SET CONF: %d\n", termios_state);
	}
}

int mb1230serial_thread_main(int argc, char *argv[])
{
	warnx("opening port %s", argv[1]);

	int serial_fd = open(argv[1], O_RDONLY | O_NOCTTY);

	if (serial_fd < 0)
	{
		err(1, "failed to open port: %s", argv[1]);
	}

	setup_serial(serial_fd);

	char R = 0;
	char raw_data[3];
	int nread = 0;
	int distance;
    int number_of_test_outputs = 100;

	struct range_finder_report data = {
		.error_count = 0,
		.type = RANGE_FINDER_TYPE_ULTRASONIC,
		.minimum_distance = MINIMAL_DISTANCE,
		.maximum_distance = MAXIMAL_DISTANCE,
		.valid = 1
	};

	orb_advert_t range_finder_pub = -1;

	thread_running = true;

	while (thread_should_run)
	{

		/*This runs at the rate of the sensors */
		struct pollfd fds[] = {
				{ .fd = serial_fd, .events = POLLIN }
		};

		/* wait for a sensor update, check for exit condition  */
		int ret = poll(fds, sizeof(fds) / sizeof(fds[0]), 1000);

		if (ret < 0)
		{
			/* poll error, ignore */
		}
		else if (ret == 0)
		{
			/* no return value, ignore */
		}
		else
		{
			if (fds[0].revents & POLLIN)
			{
				if (R != 'R')
				{
					read(serial_fd, &R, sizeof(R));
				}
				else
				{
					nread += read(serial_fd, &raw_data[nread], sizeof(raw_data) - nread);
					if (nread == 3)
					{
						nread = 0;
						R = 0;

						distance =
								(raw_data[0] - '0') * 100 +
								(raw_data[1] - '0') * 10 +
								(raw_data[2] - '0');

                        data.timestamp = hrt_absolute_time();
                        //fprintf(stderr,"[driver] %.3f < dist:%.3f > %.3f\n", (double)(MAXIMAL_DISTANCE * 100),
                        //        (double)distance,
                        //        (double)(MINIMAL_DISTANCE * 100));
                            if (distance > (MINIMAL_DISTANCE * 100) && distance < (MAXIMAL_DISTANCE * 100)) {
							data.distance = (float)distance * DISTANCE_SCALE;
                            data.valid = 1;
                            
                            //printf("Sonar %s %s %s: %.3fcm\n", raw_data[0], raw_data[1], raw_data[2], (double)data.distance);

							if (range_finder_pub > 0) {
								orb_publish(ORB_ID(sensor_range_finder), range_finder_pub, &data);
							} else {
								range_finder_pub = orb_advertise(ORB_ID(sensor_range_finder), &data);
							}
						}
                        else {
                            // Sonar is invalid, send 0xFFFF as data and invalid state
                            data.distance = 0xFFFF;
                            data.valid = 0;
                        }
                        // Test output
                        if (test_sonar) {
                            if (number_of_test_outputs > 0 ) {
                                fprintf(stdout, "[driver]Sonar distance: %.3f, valid: %d\n",(double)data.distance, data.valid);
                                number_of_test_outputs--;
                            }
                            else {
                                test_sonar = false;
                                number_of_test_outputs = 100;
                            }
                        }
					}
				}
			}
		}
	}

	warnx("exiting");
	thread_running = false;

	fflush(stdout);
	return 0;
}

