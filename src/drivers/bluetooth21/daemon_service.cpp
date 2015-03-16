#include <nuttx/config.h>
#include <sched.h>
#include <sys/types.h> // main_t
#include <systemlib/systemlib.h> // task_spawn_cmd

#include <cstdio>
#include <unistd.h>

#include "daemon.hpp"
#include "io_tty.hpp"
#include "laird/configure.hpp"
#include "laird/service_state.hpp"
#include "laird/service_io.hpp"
#include "unique_file.hpp"
#include "util.hpp"

#include "read_write_log.hpp"

namespace BT
{
namespace Daemon
{
namespace Service
{

constexpr int POLL_ms = 5 /*ms*/;
const char PROCESS_NAME[] = "bt21_service";

static volatile bool
should_run = false;

static volatile bool
running = false;

enum class Mode : uint8_t
{
	UNDEFINED,
	ONE_CONNECT,
	LISTEN,
};

static volatile Mode
daemon_mode = Mode::UNDEFINED;

static int
daemon()
{
	using namespace BT::Service::Laird;

	running = true;
	fprintf(stderr, "%s starting ...\n", PROCESS_NAME);

	unique_file dev = tty_open("/dev/btcmd");// TODO name #define/constexpr

	DevLog log_dev(fileno(dev), 2, "bt21_io      ", "bt21_service ");
	// auto & log_dev = dev;

	ServiceState svc;
	ServiceBlockingIO<decltype(log_dev)> service_io(log_dev, svc);

	should_run = daemon_mode != Mode::UNDEFINED
		// and TODO multiplexer is set up
		and fileno(dev) > -1
		and configure_latency(service_io)
		and configure_general(service_io, daemon_mode == Mode::LISTEN);

	if (should_run) { fprintf(stderr, "%s started.\n", PROCESS_NAME); }
	else
	{
		fprintf(stderr, "%s start failed: %i %s.\n"
				, PROCESS_NAME
				, errno
				, strerror(errno)
		);
	}

	while (should_run)
	{
		sleep(1);
		// poll
		// process
		// if Mode::ONE_CONNECT and not connected { sleep, reconnect }
		// check ORB subscriptions
	}

	running = false;
	fprintf(stderr, "%s stopped.\n", PROCESS_NAME);
	return 0;
}

bool
is_running() { return running; }

void
report_status(FILE * fp)
{
	fprintf(fp, "%s %s.\n"
		, PROCESS_NAME
		, is_running() ? "is running" : "is NOT running"
	);
}

void
start(const char mode[])
{
	if (running)
		return;

	if (streq(mode, "one-connect")) { daemon_mode = Mode::ONE_CONNECT; }
	else if (streq(mode, "listen")) { daemon_mode = Mode::LISTEN; }
	else { return; }

	task_spawn_cmd(PROCESS_NAME,
			SCHED_DEFAULT,
			SCHED_PRIORITY_DEFAULT,
			CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
			(main_t)daemon,
			nullptr);
}

void
request_stop()
{
	should_run = false;
	printf("%s: Service stop requested.\n", PROCESS_NAME);
}

}
// end of namespace Service
}
// end of namespace Daemon
}
// end of namespace BT
