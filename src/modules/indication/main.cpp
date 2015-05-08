extern "C" __EXPORT int main(int argc, const char * argv[]);

#include <nuttx/config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <systemlib/systemlib.h>

#include "leds.hpp"
#if CONFIG_ARCH_BOARD_AIRLEASH
# include "status_leash.hpp"
#else
# error Only AirLeash board is supported.
#endif

namespace indication
{

static bool daemon_should_run = false;
static bool daemon_running = false;

static int
daemon(int argc, char *argv[])
{
	daemon_running = true;
	fprintf(stderr, "%s has started.\n", argv[0]);

	leds::set_default();
	status::init();

	while (daemon_should_run)
	{
		hrt_abstime now = hrt_absolute_time();
		status::update(now);
		leds::update();
		usleep(1000000u / LED_TOGGLE_MAX_Hz);
	}

	leds::set_default();
	status::done();

	daemon_running = false;
	fprintf(stderr, "%s has stopped.\n", argv[0]);

	return 0;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{ std::fprintf(stderr, "Usage: %s start|stop|status\n\n", name); }

} // end of anonymous namespace

int
main(int argc, const char * argv[])
{
	using namespace indication;

	if (streq(argv[1], "start"))
	{
		if (daemon_running)
		{
			fprintf(stderr, "%s is already running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = true;
		task_spawn_cmd(argv[0],
				SCHED_DEFAULT,
				SCHED_PRIORITY_DEFAULT,
				CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
				daemon,
				argv);
	}
	else if (streq(argv[1], "status"))
	{
		if (daemon_running)
		{
			printf("%s is running.\n", argv[0]);
			leds::status();
		}
		else { printf("%s is NOT running.\n", argv[0]); }
		printf("\n");
	}
	else if (streq(argv[1], "stop"))
	{
		if (not daemon_running)
		{
			fprintf(stderr, "%s is NOT running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = false;
	}
	else if (argc == 4 and (streq(argv[1], "led-once") or streq(argv[1], "led-repeat")))
	{
		bool once = streq(argv[1], "led-once");
		unsigned n = strtoul(argv[2], 0, 0);
		uint32_t p = strtoul(argv[3], 0, 0);
		if (once)
			leds::set_pattern_once(n, p);
		else
			leds::set_pattern_repeat(n, p);
	}
	else
	{
		usage(argv[0]);
		return 1;
	}
	return 0;
}
