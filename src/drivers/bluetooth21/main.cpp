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
	fprintf(stderr, "Usage: %s start tty\n", name);
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
	using namespace BT::Daemon;

	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "start") and argc == 3)
	{
		printf("%s: Starting...\n", argv[0]);

		if (Multiplexer::is_running())
			fprintf(stderr
				, "%s: Multiplexer is already running.\n"
				, argv[0]
			);
		else
			Multiplexer::start(argv[2]);

		sleep(1);
		if (Multiplexer::is_running())
		{
			printf("%s: OK, started.\n", argv[0]);
		}
		else
		{
			printf("%s: Start failed.\n", argv[0]);
			return 1;
		}
	}
	else if (streq(argv[1], "status") and argc == 2)
	{
		Multiplexer::report_status(stdout);
	}
	else if (streq(argv[1], "stop") and argc == 2)
	{
		if (Multiplexer::is_running()) { Multiplexer::request_stop(); }
		else { Multiplexer::report_status(stderr); }

		while (Multiplexer::is_running())
		{
			fputc('.', stderr);
			fflush(stderr);
		}
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	return 0;
}
