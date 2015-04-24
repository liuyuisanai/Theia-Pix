#include <nuttx/config.h>
#include <sched.h>
#include <sys/types.h> // main_t
#include <systemlib/systemlib.h> // task_spawn_cmd

#include <cstdio>

#include "daemon.hpp"

namespace BT
{
namespace Daemon
{
namespace Main
{

constexpr useconds_t
WAIT_PERIOD_us = 100*1000 /*us*/;

const char
PROCESS_NAME[] = "bt21_main";

static volatile bool
should_run = false;

static volatile bool
running = false;

static volatile bool
started = false;

static int
daemon(int argc, const char * const argv[])
{
	running = true;
	started = false;

	printf("%s: Starting...\n", PROCESS_NAME);

	should_run = Multiplexer::start(argv[2]);
	while (should_run)
	{
		usleep(WAIT_PERIOD_us);
		bool wait =      Multiplexer::is_running()
			 and not Multiplexer::has_started();
		if (not wait) { break; }
	}

	should_run = Multiplexer::is_running()
		and Service::start(argv[3], argc > 4 ? argv[4] : nullptr);

	while (should_run)
	{
		usleep(WAIT_PERIOD_us);
		bool wait =      Multiplexer::is_running()
			 and not Multiplexer::has_started()
			 and     Service::is_running()
			 and not Service::has_started();
		if (not wait) { break; }
	}

	running = should_run = Multiplexer::is_running()
				and Service::is_running();
	if (should_run)
		printf("%s: OK, started.\n", PROCESS_NAME);
	else
	{
		printf("%s: Start failed.\n", PROCESS_NAME);
		return 1;
	}

	started = true;
	do
	{
		usleep(WAIT_PERIOD_us);
		bool wait = Multiplexer::is_running() and Service::is_running();
		if (not wait) { break; }
	}
	while (should_run);

	Multiplexer::request_stop();
	Service::request_stop();

	Service::join();
	Multiplexer::join();

	printf("%s: Stopped.\n", PROCESS_NAME);

	running = false;
	return 0;
}

bool
is_running() { return running; }

bool
has_started() { return started; }

void
start(const char * argv[])
{
	if (running)
		return;

	if (argv == nullptr or argv[0] == nullptr)
		return;

	task_spawn_cmd(PROCESS_NAME,
			SCHED_DEFAULT,
			SCHED_PRIORITY_DEFAULT,
			STACKSIZE_DAEMON_MAIN,
			(main_t)daemon,
			argv + 1);
}

void
report_status(FILE * fp)
{
	fprintf(fp, "%s %s.\n"
		, PROCESS_NAME
		, is_running() ? "is running" : "is NOT running"
	);
	Multiplexer::report_status(fp);
	Service::report_status(fp);
}

void
request_stop()
{
	should_run = false;
	fprintf(stderr, "%s stop requested.\n", PROCESS_NAME);
}

}
// end of namespace Main
}
// end of namespace Daemon
}
// end of namespace BT
