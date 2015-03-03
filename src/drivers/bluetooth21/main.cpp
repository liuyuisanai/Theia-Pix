#include <nuttx/config.h>

#include <cstdio>
#include <cstring>

#include "daemon.hpp"
#include "io_multiplexer_global.hpp"

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{
	// TODO
	//fprintf(stderr, "Usage: %s start tty\n", name);
	fprintf(stderr, "Usage: %s start\n", name);
	fprintf(stderr, "       %s status\n", name);
	fprintf(stderr, "       %s stop\n", name);
	fprintf(stderr, "\n");
}

#ifdef MODULE_COMMAND
#define XCAT(a, b)  a ## b
#define CONCAT(a, b)  XCAT(a, b)
#define main CONCAT(MODULE_COMMAND, _main)
#endif

extern "C" __EXPORT int
main(int argc, const char * const argv[]);

int
main(int argc, const char * const argv[])
{
	using namespace BT;

	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "start") and argc == 3)
	{
		if (Daemon::Multiplexer::is_running())
		{
			fprintf(stderr, "Deamon::Multiplexer is already running.\n");
			return 1;
		}
		printf("Starting...");
		Daemon::Multiplexer::start(argv[2]);
	}
	else if (streq(argv[1], "status") and argc == 2)
	{
		if (Daemon::Multiplexer::is_running())
			printf("Daemon::Multiplexer is running.\n");
		else
			printf("Daemon::Multiplexer is NOT running.\n");
		printf("\n");
	}
	else if (streq(argv[1], "stop") and argc == 2)
	{
		if (not Daemon::Multiplexer::is_running())
		{
			fprintf(stderr, "Deamon::Multiplexer is NOT running.\n");
			return 1;
		}
		printf("Stopping...");
		Daemon::Multiplexer::request_stop();
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	return 0;
}
