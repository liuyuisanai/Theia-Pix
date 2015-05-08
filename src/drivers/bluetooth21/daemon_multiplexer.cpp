#include <nuttx/config.h>
#include <pthread.h>
#include <termios.h>

#include <cstdio>
#include <cstring>

#include "chardev.hpp"
#include "daemon.hpp"
#include "io_multiplexer_global.hpp"
#include "io_multiplexer_poll.hpp"
#include "io_tty.hpp"
#include "laird/uart.hpp"
#include "module_params.hpp"
#include "unique_file.hpp"

#include "trace.hpp"

namespace BT
{
namespace Daemon
{
namespace Multiplexer
{

constexpr int
POLL_ms = 5 /*ms*/;

const char
PROCESS_NAME[] = "bt21_io";

static volatile bool
should_run = false;

static volatile bool
running = false;

static volatile bool
started = false;

static char
dev_name[16];

static pthread_t
thread;

static pthread_attr_t
thread_attr;

static void *
daemon()
{
	pthread_setname_np(thread, PROCESS_NAME);

	running = true;
	started = false;
	fprintf(stderr, "%s starting ...\n", PROCESS_NAME);

	unique_file raw_dev(tty_open(dev_name));
	auto trace = make_trace_handle<MULTIPLEXER_TRACE>(
		MULTIPLEXER_TRACE_FILE, raw_dev, "module  ", "host    "
	);

	const bool use_ctsrts = Params::get("A_TELEMETRY_FLOW");

	should_run = (
		fileno(raw_dev) > -1
		and tty_switch_ctsrts(fileno(raw_dev), use_ctsrts)
		and Globals::Multiplexer::create()
		and CharacterDevices::register_all_devices()
	);

	if (not use_ctsrts)
		fprintf(stderr
			, "\n%s: WARNING: CTS/RTS flow control DISABLED.\n\n"
			, PROCESS_NAME
		);

	if (should_run)
	{
		auto & mp = Globals::Multiplexer::get();
		should_run = setup_serial(mp.protocol_tag, trace.dev);
	}

	if (should_run) { fprintf(stderr, "%s started.\n", PROCESS_NAME); }
	else
	{
		fprintf(stderr, "%s start failed: %s."
				, PROCESS_NAME
				, strerror(errno)
		);
	}

	started = true;
	while (should_run)
	{
		auto & mp = Globals::Multiplexer::get();
		perform_poll_io(trace.dev, mp, POLL_ms);
		if (not is_healthy(mp)) { break; }
		fsync(trace.dev);
	}

	while (should_run)
	{
		fprintf(stderr
			, "%s stopped IO processing because went unhealty.\n"
			, PROCESS_NAME
		);
		sleep(1);
	}

	CharacterDevices::unregister_all_devices();
	Globals::Multiplexer::destroy();

	started = running = false;
	fprintf(stderr, "%s stopped.\n", PROCESS_NAME);

	return nullptr;
}

bool
is_running() { return running; }

bool
has_started() { return started; }

void
report_status(FILE * fp)
{
	fprintf(fp, "%s %s.\n"
		, PROCESS_NAME
		, is_running() ? "is running" : "is NOT running"
	);
}

bool
start(const char uart_dev_name[])
{
	if (strlen(uart_dev_name) >= sizeof dev_name)
	{
		fprintf(stderr
			, "Too long path '%s', max %uchars.\n"
			, uart_dev_name
			, sizeof dev_name
		);
		return false;
	}
	std::strncpy(dev_name, uart_dev_name, sizeof dev_name);

	errno = pthread_attr_init(&thread_attr);
	if (errno != 0)
	{
		perror("Service / pthread_init");
		return false;
	}

	errno = pthread_attr_setstacksize(&thread_attr, STACKSIZE_DAEMON_IO);
	if (errno != 0)
	{
		perror("Service / pthread_attr_setstacksize");
		return false;
	}

	errno = pthread_create(
		&thread, &thread_attr, (pthread_func_t)daemon, nullptr
	);
	if (errno != 0)
	{
		perror("Multiplexer / pthread_create");
		return false;
	}

	return true;
}

void
join()
{
	pthread_join(thread, nullptr);
	pthread_attr_destroy(&thread_attr);
}

void
request_stop()
{
	should_run = false;
	fprintf(stderr, "%s stop requested.\n", PROCESS_NAME);
}

}
// end of namespace Multiplexer
}
// end of namespace Daemon
}
// end of namespace BT
