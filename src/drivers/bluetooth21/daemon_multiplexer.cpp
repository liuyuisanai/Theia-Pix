#include <nuttx/config.h>
#include <sched.h>
#include <sys/types.h> // main_t
#include <systemlib/systemlib.h> // task_spawn_cmd
#include <unistd.h> // sleep

#include <cstdio>

#include "chardev.hpp"
#include "daemon.hpp"
#include "io_multiplexer_global.hpp"
#include "io_multiplexer_poll.hpp"
#include "io_tty.hpp"
#include "unique_file.hpp"

namespace BT
{
namespace Daemon
{
namespace Multiplexer
{

static volatile bool
should_run = false;

static volatile bool
running = false;

static int
daemon(int argc, const char * const argv[])
{

	unique_file dev;

	if (argc == 2)
		dev = tty_open(argv[1]);
	else
		fprintf(stderr, "Wrong argument count.\n");

	running = should_run =
		fileno(dev) != -1
		and tty_set_speed(fileno(dev), B115200) // TODO move speed to a header
		and Globals::Multiplexer::create()
		and CharacterDevices::register_all_devices();

	if (should_run)
		fprintf(stderr, "Daemon::Multiplexer started.\n");
	else
		perror("Daemon::Multiplexer start failed");

	while (should_run)
	{
		auto & mp = Globals::Multiplexer::get();
		perform_poll_io(dev, mp, 100 /*ms*/);
	}

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

	const char * argv[] = { "bt21_io", uart_dev_name, nullptr };
	task_spawn_cmd(argv[0],
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
