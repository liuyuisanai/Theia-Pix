#include <nuttx/config.h>

#include <cstdio>
#include <cstring>

#include "daemon.hpp"
#include "io_multiplexer_global.hpp"
#include "util.hpp"

static void
usage(const char name[])
{
	fprintf(stderr,
		"Usage: %s start tty <mode>\n"
		"       %s status\n"
		"       %s stop\n"
		"Modes: \n"
		" * one-connect\n"
		" * listen\n"
		"\n"
		, name, name, name
	);
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
	using BT::streq;
	using namespace BT::Daemon;

	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "start") and argc == 4)
	{
		printf("%s: Starting...\n", argv[0]);

		if (Multiplexer::is_running())
			fprintf(stderr
				, "%s: Multiplexer is already running.\n"
				, argv[0]
			);
		else
			Multiplexer::start(argv[2]);

		if (Service::is_running())
			fprintf(stderr
				, "%s: Service is already running.\n"
				, argv[0]
			);
		else
			Service::start(argv[3]);

		sleep(1);
		if (Multiplexer::is_running() and Service::is_running())
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
		Service::report_status(stdout);
	}
	else if (streq(argv[1], "stop") and argc == 2)
	{
		if (Multiplexer::is_running())
			Multiplexer::request_stop();
		else
			Multiplexer::report_status(stderr);

		if (Service::is_running())
			Service::request_stop();
		else
			Service::report_status(stderr);

		while (Multiplexer::is_running() and Service::is_running())
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
