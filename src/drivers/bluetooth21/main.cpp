#include <nuttx/config.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "daemon.hpp"
#include "io_multiplexer_global.hpp"
#include "util.hpp"

constexpr useconds_t
WAIT_PERIOD_us = 100*1000 /*us*/;

static void
usage(const char name[])
{
	fprintf(stderr, "Usage:\n"
		"\t%s start tty factory-param\n"
		"\t%s           listen\n"
		"\t%s           one-connect address\n"
		"\t%s           loopback-test\n"
		"\t%s status\n"
		"\t%s stop\n"
		"\n"
		, name, name, name, name, name, name
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

	if (streq(argv[1], "start") and argc >= 4)
	{
		printf("%s: Starting...\n", argv[0]);

		if (Multiplexer::is_running())
			fprintf(stderr, "%s is *already* running.\n",
					Multiplexer::PROCESS_NAME);
		else
		{
			Multiplexer::start(argv[2]);

			bool wait;
			do
			{
				usleep(WAIT_PERIOD_us);
				wait =      Multiplexer::is_running()
					and not Multiplexer::has_started();
			}
			while (wait);
		}

		if (Service::is_running())
			fprintf(stderr, "%s is *already* running.\n",
					Service::PROCESS_NAME);
		else if (Multiplexer::is_running())
			Service::start(argv[3], argc > 4 ? argv[4] : nullptr);

		bool wait;
		do
		{
			usleep(WAIT_PERIOD_us);
			wait =      Multiplexer::is_running()
				and not Multiplexer::has_started()
				and Service::is_running()
				and not Service::has_started();
		}
		while (wait);

		if (Multiplexer::is_running() and Service::is_running())
			printf("%s: OK, started.\n", argv[0]);
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
			usleep(WAIT_PERIOD_us);
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
