#include <nuttx/config.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <termios.h>

#include <systemlib/systemlib.h>

#include "dispatch.hpp"
#include "io_blocking.hpp"
#include "io_tty.hpp"
#include "read_write_log.hpp"
#include "unique_file.hpp"

namespace
{

static bool daemon_should_run = false;
static bool daemon_running = false;

static int
daemon(int argc, char *argv[])
{
	unique_file d = tty_open(argv[1]);
	bool ok = fileno(d) != -1
		and tty_set_speed(fileno(d), B9600)
		and tty_use_ctsrts(fileno(d));
	if (not ok) { return 1; }

	DevLog log (d.get(), 2, "read  ", "write ");
	auto f = make_it_blocking< 1000/*ms*/ >(log);

	daemon_running = true;
	fprintf(stderr, "%s has started.\n", argv[0]);

	FileRequestHandler file_requests;
	ReceiveFileHandle receive_file;

	fprintf(stderr, "Processing");

	while (daemon_should_run)
		process_one_command(f, file_requests, receive_file);

	daemon_running = false;

	fprintf(stderr, "%s has stopped.\n", argv[0]);
	return 0;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{
	std::fprintf(stderr,
		"Usage: %s start TTY\n"
		"       %s stop\n"
		"       %s status\n"
		"\n",
		name, name, name
	);
}

} // end of anonymous namespace

int
main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (argc == 3 and streq(argv[1], "start"))
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
				argv + 1);
	}
	else if (argc == 2 and streq(argv[1], "status"))
	{
		if (daemon_running) { printf("%s is running.\n", argv[0]); }
		else { printf("%s is NOT running.\n", argv[0]); }
	}
	else if (argc == 2 and streq(argv[1], "stop"))
	{
		if (not daemon_running)
		{
			fprintf(stderr, "%s is NOT running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = false;
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	fprintf(stderr, "main() is returning 0\n");
	return 0;
}
