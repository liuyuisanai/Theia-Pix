#include <nuttx/config.h>
#include <sched.h>
#include <sys/types.h> // main_t
#include <systemlib/systemlib.h> // task_spawn_cmd

#include <cstdio>

#include "chardev.hpp"
#include "daemon.hpp"
#include "io_multiplexer_global.hpp"
#include "io_multiplexer_poll.hpp"
#include "io_tty.hpp"
#include "unique_file.hpp"

#include "read_write_log.hpp"

namespace BT
{
namespace Daemon
{
namespace Multiplexer
{

constexpr int POLL_ms = 5 /*ms*/;
const char PROCESS_NAME[] = "bt21_io";

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
	fprintf(stderr, "%s starting ...\n", PROCESS_NAME);

	unique_file dev;

	if (argc == 2)
		dev.set(tty_open(argv[1]));
	else
		fprintf(stderr, "%s: wrong argument count.\n", PROCESS_NAME);

	should_run = (
		fileno(dev) > -1
		and tty_set_speed(fileno(dev), B115200) // TODO move speed to a header
		and Globals::Multiplexer::create()
		and CharacterDevices::register_all_devices()
	);

	if (should_run) { fprintf(stderr, "%s started.\n", PROCESS_NAME); }
	else
	{
		fprintf(stderr, "%s start failed: %s."
				, PROCESS_NAME
				, strerror(errno)
		);
	}

	//DevLog log_dev(fileno(dev), 2, "module  ", "host    ");
	auto & log_dev = dev;

	started = true;
	while (should_run)
	{
		auto & mp = Globals::Multiplexer::get();
		perform_poll_io(log_dev, mp, POLL_ms);
		if (not is_healthy(mp)) { break; }
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
	return 0;
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

void
start(const char uart_dev_name[])
{
	if (running)
		return;

	const char * argv[] = { uart_dev_name, nullptr };
	task_spawn_cmd(PROCESS_NAME,
			SCHED_DEFAULT,
			SCHED_PRIORITY_DEFAULT,
			STACKSIZE_DAEMON_IO,
			(main_t)daemon,
			argv);
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
