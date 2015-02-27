#include <nuttx/config.h>
#include <sched.h>
#include <sys/types.h> // main_t
#include <systemlib/systemlib.h> // task_spawn_cmd
#include <unistd.h> // sleep

#include <cstdio>

#include "chardev.hpp"
#include "daemon.hpp"
#include "io_multiplexer_global.hpp"

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
	running = should_run =
		Globals::Multiplexer::create()
		and CharacterDevices::register_all_devices();

	if (should_run) { fprintf(stderr, "Daemon::Multiplexer started.\n"); }

	while (should_run)
	{
		sleep(1);
		// TODO real io code
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

	const char * argv[2] = { uart_dev_name, nullptr };
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
