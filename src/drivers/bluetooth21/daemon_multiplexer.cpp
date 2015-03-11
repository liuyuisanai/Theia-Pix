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

static volatile bool
should_run = false;

static volatile bool
running = false;

static int
daemon(int argc, const char * const argv[])
{

	unique_file dev;

	if (argc == 2)
		dev.set(tty_open(argv[1]));
	else
		fprintf(stderr, "Wrong argument count.\n");

	running = should_run = (
		fileno(dev) > -1
		and tty_set_speed(fileno(dev), B115200) // TODO move speed to a header
		and Globals::Multiplexer::create()
		and CharacterDevices::register_all_devices()
	);

	if (should_run)
		fprintf(stderr, "Daemon::Multiplexer started.\n");
	else
		perror("Daemon::Multiplexer start failed");

	auto & mp = Globals::Multiplexer::get();
	//DevLog log_dev(fileno(dev), 2, "module  ", "host    ");
	auto & log_dev = dev;
	while (should_run) { perform_poll_io(log_dev, mp, POLL_ms); }

	CharacterDevices::unregister_all_devices();
	Globals::Multiplexer::destroy();

	running = false;
	return 0;
}

bool
is_running() { return running; }

void
start(const char uart_dev_name[])
{
	if (running)
		return;

	const char * argv[] = { uart_dev_name, nullptr };
	task_spawn_cmd("bt21_io",
			SCHED_DEFAULT,
			SCHED_PRIORITY_DEFAULT,
			CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
			(main_t)daemon,
			argv);
}

void
request_stop() { should_run = false; }

}
// end of namespace Multiplexer
}
// end of namespace Daemon
}
// end of namespace BT
