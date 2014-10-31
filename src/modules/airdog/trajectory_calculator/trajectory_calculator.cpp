#include <nuttx/config.h>

#include <geo/geo.h>
#include <math.h>
#include <mathlib/math/filter/LowPassFilter.hpp>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <systemlib/err.h>
#include <systemlib/param/param.h>
#include <systemlib/systemlib.h>
#include <uORB/uORB.h>
#include <uORB/topics/home_position.h>
#include <uORB/topics/trajectory.h>
#include <uORB/topics/vehicle_local_position.h>

namespace trajectory_calculator {

// Global variables, controlling threaded execution
static bool g_thread_continue = false;
static bool g_thread_running = false;

// Helper class for position analysis.
// TODO! Conisider migrating to function
class PointAnalyzer {
private:
	// TODO! Consider migrating to a vector filtering
	math::LowPassFilter<float> x_filter, y_filter;
	float radius_sq;
	float center_x, center_y;
public:
	PointAnalyzer(float filter_cutoff, float init_radius) :
				x_filter(filter_cutoff),
				y_filter(filter_cutoff),
				radius_sq(init_radius*init_radius),
				center_x(0.0f/0.0f),
				center_y(center_x) {
	}
	~PointAnalyzer() {}

	// Analyze provided point, filter its x and y, update center, if needed and return point type
	uint8_t analyze(vehicle_local_position_s &point) {
		float x, y;

		point.x = x_filter.apply(point.timestamp, point.x);
		point.y = y_filter.apply(point.timestamp, point.y);
		if (!isfinite(center_x) || !isfinite(center_y)) {
			center_x = point.x;
			center_y = point.y;
		}
		x = point.x - center_x;
		y = point.y - center_y;
		// TODO! Consider variable radius based on speed
		if ((x*x + y*y) < radius_sq) {
			return 0; // still point
		}
		else {
			center_x = point.x;
			center_y = point.y;
			return 1; // moving point
		}
	}
};

// Thread entry point. Calculates trajectory points and publishes them
int calculate(int argc, char *argv[]) {
	// TODO! Parameters instead of constants
	float cutoff, radius;
	if (param_get(param_find("AIRD_TRAJ_CUT"), &cutoff) != 0 ||
			param_get(param_find("AIRD_TRAJ_RAD"), &radius) != 0) {
		warnx("Could not read parameters!");
		return -1;
	}

	// Prepare pollers
	int pos_topic = orb_subscribe(ORB_ID(vehicle_local_position));
	pollfd poll_data;
	poll_data.fd = pos_topic;
	poll_data.events = POLLIN;
	int home_topic = orb_subscribe(ORB_ID(home_position));
	int res;

	orb_advert_t trajectory_pub = -1;
	trajectory_s trajectory_data;

	// TODO! We might be to the party too early - consider waiting for the first publication
	vehicle_local_position_s local_position = {};
	home_position_s home;

	bool updated;
	// We consider home position secondary, thus we need to initialize it with correct data
	orb_copy(ORB_ID(home_position), home_topic, &home);

	// Projection vars
	struct map_projection_reference_s ref_point;
	double est_lat, est_lon;
	// Latitude and longitude in ref_point are converted, but we need to compare with local refs
	double ref_lat = 0.0f/0.0f, ref_lon = ref_lat;

	// Filtering object
	PointAnalyzer analyzer(cutoff, radius);

	g_thread_running = true;
	while (g_thread_continue) {
		// pace the daemon by global position topic, 100 HZ with a bit of overhead
		res = poll(&poll_data, 1, 15);
		if (res == 1) {
			orb_copy(ORB_ID(vehicle_local_position), pos_topic, &local_position);
			orb_check(home_topic, &updated);
			if (updated) {
				orb_copy(ORB_ID(home_position), home_topic, &home);
			}

			// TODO! Currently we trust local_position topic to not have incorrect refs
			// TODO! Use xy_global to check for valid refs
			if (local_position.ref_lat != ref_lat || local_position.ref_lon != ref_lon) {
				map_projection_init(&ref_point, local_position.ref_lat, local_position.ref_lon);
				ref_lat = local_position.ref_lat;
				ref_lon = local_position.ref_lon;
			}

			// Filters and modifies local position while he's at it
			trajectory_data.point_type = analyzer.analyze(local_position);

			// TODO! Raw fix to avoid publishing a still point on top of a valid point, thus Mavlink will get only still points
			if (trajectory_data.point_type != 0) {
				map_projection_reproject(&ref_point, local_position.x, local_position.y, &est_lat, &est_lon);

				trajectory_data.alt = local_position.ref_alt - local_position.z;
				trajectory_data.lat = est_lat;
				trajectory_data.lon = est_lon;
				trajectory_data.vel_d = local_position.vz;
				trajectory_data.vel_e = local_position.vy;
				trajectory_data.vel_n = local_position.vx;
				trajectory_data.relative_alt = home.alt - trajectory_data.alt;
				trajectory_data.timestamp = local_position.timestamp;
				trajectory_data.heading = _wrap_2pi(local_position.yaw);

				// TODO! Limit publishing frequency to Mavlink stream frequency
				if (trajectory_pub < 0) {
					trajectory_pub = orb_advertise(ORB_ID(trajectory), &trajectory_data);
				} else {
					orb_publish(ORB_ID(trajectory), trajectory_pub, &trajectory_data);
				}
			}
		}
		else {
			warnx("Poll error!");
		}
	}

	close(pos_topic);
	close(home_topic);
	close(trajectory_pub);
	g_thread_running = false;
	return 0;
}

// Helper function to start the thread
int start() {
	if (g_thread_running) {
		return -1;
	}
	g_thread_continue = true;
	// Beware of the stack size. With 500 bytes the process just hangs!
	if (0 >= task_spawn_cmd("trajectory_daemon",
			SCHED_DEFAULT,SCHED_PRIORITY_DEFAULT, 2000,
			calculate,
			(const char**) NULL)){
		g_thread_continue = false;
		return -2;
	}
	// TODO! Consider checking if the thread successfully started
	return 0;
}

// Helper function to stop the thread
int stop() {
	int i = 0;

	if (!g_thread_running) {
		return -1;
	}
	g_thread_continue = false;
	// Wait for the thread to actually stop, pausing the caller
	while (g_thread_running) {
		// Wait max of 10 seconds
		if (i++ > 10) {
			return -2;
		}
		sleep(1);
	}
	return 0;
}

} // End trajectory_calculator namespace

inline void usage(const char *progname) {
	printf("Usage: %s {start|stop|status}\n", progname);
}

extern "C" __EXPORT int trajectory_calculator_main(int argc, char *argv[]) {
	using namespace trajectory_calculator;

	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	if (strcmp(argv[1], "start") == 0) {
		switch (start()) {
		case 0:
			warnx("Started.");
			break;
		case -1:
			errx(1, "Already running!");
			break;
		default:
			errx(1, "Failed to start the calculator!");
			break;
		}
	}
	else if (strcmp(argv[1], "stop") == 0) {
		switch (stop()) {
		case 0:
			warnx("Stopped.");
			break;
		case -1:
			errx(1, "Not running!");
			break;
		default:
			errx(1, "Failed to stop the calculator!");
			break;
		}
	}
	else if (strcmp(argv[1], "status") == 0) {
		if (g_thread_running == 0) {
			warnx("Not running.");
		}
		else {
			warnx("Running.");
		}
	}
	else {
		warnx("Unknown argument: '%s'", argv[1]);
		usage(argv[0]);
		return 1;
	}
	return 0;
}
