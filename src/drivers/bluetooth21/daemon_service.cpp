#include <nuttx/config.h>
#include <pthread.h>

#include <cstdio>
#include <unistd.h>

#include "daemon.hpp"
#include "device_connection_map.hpp"
#include "factory_addresses.hpp"
#include "io_multiplexer_flags.hpp"
#include "io_multiplexer_global.hpp"
#include "io_tty.hpp"
#include "laird/configure.hpp"
#include "laird/service_io.hpp"
#include "laird/service_state.hpp"
#include "unique_file.hpp"
#include "util.hpp"

#include "trace.hpp"

namespace BT
{
namespace Daemon
{
namespace Service
{

const char
PROCESS_NAME[] = "bt21_service";

static volatile bool
should_run = false;

static volatile bool
running = false;

static volatile bool
started = false;

enum class Mode : uint8_t
{
	UNDEFINED,
	ONE_CONNECT,
	LISTEN,
};

static volatile Mode
daemon_mode = Mode::UNDEFINED;

static Address6
connect_address;

static pthread_t
thread;

static pthread_attr_t
thread_attr;

template <typename ServiceIO, typename ServiceState>
static void
synced_loop(MultiPlexer & mp, ServiceIO & service_io, ServiceState & svc)
{
	using namespace BT::Service::Laird;

	auto & dev = service_io.dev;
	while (should_run and not module_rebooted(svc.sync))
	{
		wait_process_event(service_io);
		set_xt_ready_mask(mp, svc.flow.xt_mask);

		if (svc.conn.changed)
		{
			dbg("Connect/disconnect.\n");
			update_connections(mp, svc.conn.channels_connected);
			svc.conn.changed = false;
		}

		dbg("Connections waiting %i, total count %u.\n"
			, no_single_connection(svc.conn)
			, count_connections(svc.conn)
		);
		if (no_single_connection(svc.conn))
		{
			if (daemon_mode == Mode::ONE_CONNECT
			and allowed_connection_request(svc.conn)
			) {
				request_connect(dev, svc.conn, connect_address);
				dbg("Request connect.\n");
			}
		}
		else { /* TODO request rssi */ }
	}
}

static void *
daemon()
{
	using namespace BT::Service::Laird;

	pthread_setname_np(thread, PROCESS_NAME);

	running = true;
	started = false;
	fprintf(stderr, "%s starting ...\n", PROCESS_NAME);

	unique_file raw_dev = tty_open("/dev/btcmd");// TODO name #define/constexpr
	auto trace = make_trace_handle<SERVICE_TRACE>(
		SERVICE_TRACE_FILE, raw_dev, "bt21_io  ", "bt21_svc "
	);

	auto & mp = Globals::Multiplexer::get();
	ServiceState svc;
	auto service_io = make_service_io(trace.dev, svc);
	should_run = (daemon_mode != Mode::UNDEFINED
		and fileno(raw_dev) > -1
		and sync_soft_reset(service_io, svc.sync)
		and configure_factory(service_io)
		and configure_before_reboot(service_io)
		and sync_soft_reset(service_io, svc.sync)
		and configure_after_reboot(service_io)
		and dump_s_registers(service_io)
	);

	if (should_run) { fprintf(stderr, "%s started.\n", PROCESS_NAME); }
	else
	{
		fprintf(stderr, "%s start failed: %i %s.\n"
				, PROCESS_NAME
				, errno
				, strerror(errno)
		);
	}

	started = true;

	do
	{
		synced_loop(mp, service_io, svc);

		// We'll get here on stop request or on module reboot
		bool ok = ( should_run
			and configure_after_reboot(service_io)
			and renew_after_reboot(service_io, svc.conn)
		);

		if (not ok) { break; }

		// Assume we are syncronized again
		set_in_sync(svc.sync);
	}
	while (true);

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
start(const char mode[], const char addr_no[])
{
	dbg("%s Service::start(%s, %s).\n", PROCESS_NAME, mode, addr_no);
	daemon_mode = Mode::UNDEFINED;
	if (streq(mode, "factory-param"))
	{
		uint32_t i = Params::get("A_BT_CONNECT_TO");
		if (i < n_factory_addresses)
		{
			daemon_mode = Mode::ONE_CONNECT;
			connect_address = factory_addresses[i];
		}
		else
		{
			daemon_mode = Mode::LISTEN;
		}
	}
	else if (streq(mode, "one-connect"))
	{
		uint32_t i;
		if (not addr_no)
			fprintf(stderr, "%s: factory_address_no required.\n",
				PROCESS_NAME);
		else if (not parse_uint32(addr_no, i))
			fprintf(stderr, "%s: invalid factory_address_no %s.\n",
				PROCESS_NAME, addr_no);
		else if (i < n_factory_addresses)
		{
			daemon_mode = Mode::ONE_CONNECT;
			connect_address = factory_addresses[i];
		}
		else
			fprintf(stderr
				, "%s: invalid factory_address_no %i max %i.\n"
				, PROCESS_NAME
				, i
				, n_factory_addresses - 1
			);

	}
	else if (streq(mode, "listen"))
	{
		daemon_mode = Mode::LISTEN;
	}
	else if (streq(mode, "loopback-test"))
	{
		fprintf(stderr, "The '%s' mode doesn't use %s.\n"
			, mode
			, PROCESS_NAME
		);
	}
	else
	{
		fprintf(stderr, "%s: Invaid mode: %s.\n"
			, PROCESS_NAME
			, mode
		);
	}

	dbg("%s daemon mode %u.\n", PROCESS_NAME, daemon_mode);

	if (daemon_mode == Mode::UNDEFINED)
		return false;

	if (daemon_mode == Mode::ONE_CONNECT)
		fprintf(stderr
			, "%s: mode one-connect to " Address6_FMT ".\n"
			, PROCESS_NAME
			, Address6_FMT_ITEMS(connect_address)
		);
	else
		fprintf(stderr, "%s: mode listen.\n", PROCESS_NAME);

	errno = pthread_attr_init(&thread_attr);
	if (errno != 0)
	{
		perror("Service / pthread_init");
		return false;
	}

	errno = pthread_attr_setstacksize(
			&thread_attr, STACKSIZE_DAEMON_SERVICE
	);
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
		perror("Service / pthread_create");
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
// end of namespace Service
}
// end of namespace Daemon
}
// end of namespace BT
