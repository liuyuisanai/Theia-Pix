#include <nuttx/config.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <cstdio>
#include <unistd.h>

#include <drivers/drv_tone_alarm.h>
#include <drivers/drv_hrt.h>

#include "daemon.hpp"
#include "device_connection_map.hpp"
#include "factory_addresses.hpp"
#include "io_multiplexer_flags.hpp"
#include "io_multiplexer_global.hpp"
#include "svc_globals.hpp"
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

using namespace BT::Service;

const char
PROCESS_NAME[] = "bt21_service";

hrt_abstime last_pairing_check = 0;

hrt_abstime button_on_t;
hrt_abstime button_off_t;

static volatile bool
should_run = false;

static volatile bool
running = false;

static volatile bool
started = false;

enum class Mode : uint8_t
{
	UNDEFINED,
	LISTEN,
	ONE_CONNECT,
};

static volatile Mode
daemon_mode = Mode::UNDEFINED;

bool 
valid_connect_address = false;

static Address6
connect_address;

static Address6
pairing_address;

static bool
pairing_req = false;

bool
pairing_done_this_cycle = false;

static bool
paired = false;

static pthread_t
thread;

static pthread_attr_t
thread_attr;

#define _BLUETOOTH21_BASE		0x2d00

#define PAIRING_ON			_IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF			_IOC(_BLUETOOTH21_BASE, 1)
#define PAIRING_TOGGLE		_IOC(_BLUETOOTH21_BASE, 2)

bool
check_pairing_enabled(){
    return Globals::Service::get_pairing_status();
}


template <typename ServiceIO>
static void
inquiry_loop(ServiceIO & service_io, InquiryState & inq)
{
	unique_file tones = open(TONEALARM_DEVICE_PATH, O_WRONLY);

	dbg("Discovery started.\n");
	while (should_run
	and inquiry(service_io, inq)
	and not closest_is_obvious(inq)
	) {
		dbg("Discovered %u BT devices of class 0x%06x.\n"
			, inq.n_results
			, (unsigned) inq.DRONE_COD
		);

		// TODO move tones to modules/indication
		int tune;
		if (inq.n_results == 0)
			tune = TONE_NOTIFY_NEUTRAL_TUNE;
		else
			tune = TONE_NOTIFY_NEGATIVE_TUNE;
		ioctl(fileno(tones), TONE_SET_ALARM, tune);
	}

	if (not closest_is_obvious(inq))
		ioctl(fileno(tones), TONE_SET_ALARM, TONE_ERROR_TUNE);

	dbg("Discovery finished.\n");
}

template <typename ServiceIO, typename ServiceState>
static void
pairing_loop(ServiceIO & service_io, ServiceState & svc){

    dbg("pairing_loop() starting.\n");

    drop_trusted_db(service_io);

    svc.pairing.pairing_active = true;

    if (daemon_mode == Mode::LISTEN) {

        switch_discoverable(service_io, true);

        svc.pairing.pairing_initiator = false;

        while (svc.pairing.pairing_active){

            paired = pair(service_io);
            // svc.pairing.pairing_active = check_pairing_enabled();
            svc.pairing.pairing_active = Globals::Service::get_pairing_status();
        }

        switch_discoverable(service_io, false);

    } else if  (daemon_mode == Mode::ONE_CONNECT) {

        bool pairing_device_found = false;

        while (svc.pairing.pairing_active) {

            fsync(service_io.dev);
            inquiry_loop(service_io, svc.inq);
            if (closest_is_obvious(svc.inq)) {
                pairing_address = closest_address(svc.inq);
                pairing_device_found = true;

                dbg("Pairing device found. Closest is obvious. \n");

                break;
            }
            svc.pairing.pairing_active = check_pairing_enabled();
        }

        if (pairing_device_found) {

            // TODO: Success sounds and actions

            svc.pairing.addr = pairing_address;
            svc.pairing.pairing_initiator = true;

            paired = pair(service_io);

            if (paired) {
                
                connect_address = pairing_address;
                valid_connect_address = true;

            } else {

                // TODO: Fail actions
            
            }

        } else {

            // TODO: Fail sounds and actions
        
        }

        Globals::Service::turn_pairing_off();

    }

}

template <typename ServiceIO, typename ServiceState>
static void
synced_loop(MultiPlexer & mp, ServiceIO & service_io, ServiceState & svc)
{
	using namespace Laird;

	auto & dev = service_io.dev;
	while (should_run and not module_rebooted(svc.sync))
	{

		wait_process_event(service_io);
		set_xt_ready_mask(mp, svc.flow.xt_mask);

        if (pairing_req) {
        // Check every 1 scond if pairing is enabled

            if (hrt_elapsed_time(&last_pairing_check) > 1000000){

                last_pairing_check = hrt_absolute_time();

                if (check_pairing_enabled()) {
                    if (!pairing_done_this_cycle) {

                        dbg("Pairing activated\n");

                        pairing_loop(service_io, svc);

                        pairing_done_this_cycle = true;

                        dbg("Pairing done\n");

                    }
                } else {

                    pairing_done_this_cycle = false;

                }
            }
        } 

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
            and valid_connect_address
			) {
				request_connect(dev, svc.conn, connect_address);
				dbg("Request connect.\n");
			}
		}
		else { /* TODO request rssi */ }

		fsync(service_io.dev);
	}
}




template <typename ServiceIO, typename ServiceState>
static void
keep_sync_loop(MultiPlexer & mp, ServiceIO & service_io, ServiceState & svc)
{
	using namespace Laird;

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
}

static void *
daemon()
{
	using namespace BT::Service::Laird;

	pthread_setname_np(thread, PROCESS_NAME);

	running = true;
	started = false;

	fprintf(stderr, "%s starting ...\n", PROCESS_NAME);

	unique_file raw_dev = tty_open("/dev/btservice");// TODO name #define/constexpr
	auto trace = make_trace_handle<SERVICE_TRACE>(
		SERVICE_TRACE_FILE, raw_dev, "bt21_io  ", "bt21_svc "
	);

    Globals::Service::turn_pairing_off();

	auto & mp = Globals::Multiplexer::get();
	ServiceState svc;
	auto service_io = make_service_io(trace.dev, svc);
	should_run = (daemon_mode != Mode::UNDEFINED
		and fileno(raw_dev) > -1
		and sync_soft_reset(service_io, svc.sync)
		and configure_before_reboot(service_io)
		and sync_soft_reset(service_io, svc.sync)
		and configure_after_reboot(service_io)
		and dump_s_registers(service_io)
	);

    if (should_run && !pairing_req) {
		should_run = configure_factory(service_io)
            and switch_discoverable(service_io, true);
    }

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

    if (pairing_req) {
    
        // Get connect address form trust_db if we paired before.
        if (trusted_db_record_count_get(service_io, 1) > 0) {
            connect_address = get_trusted_address(service_io, 1, 1);
            valid_connect_address = true;
        }
    
    } else {

        if (should_run)
        {

            fsync(service_io.dev);
            inquiry_loop(service_io, svc.inq);
            should_run = should_run and closest_is_obvious(svc.inq);
            if (should_run) {
                connect_address = closest_address(svc.inq);
                valid_connect_address = true;
            }
        } 
    }
    
	if (should_run)
	{
		fsync(service_io.dev);
		keep_sync_loop(mp, service_io, svc);
	}

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

    pairing_req = Params::get("A_BT_PAIR_REQ");

	daemon_mode = Mode::UNDEFINED;

    if (pairing_req){

        // Discoverable off 

        dbg("%s Pairing is required. .\n", PROCESS_NAME);
        
        if (streq(mode, "one-connect")) {

            daemon_mode = Mode::ONE_CONNECT;

            // Hardcoded times when pairing is enabled and then disabled for testing purposes
            button_on_t = hrt_absolute_time() + 1e6 * 20.0;
            button_off_t = hrt_absolute_time() + 1e6 * 150.0;

        } else if (streq(mode, "listen")) {

            daemon_mode = Mode::LISTEN;

            button_on_t = hrt_absolute_time() + 1e6 * 50.0;
            button_off_t = hrt_absolute_time() + 1e6 * 150.0;
            
        
        } else {

            fprintf(stderr, "%s: Invaid mode: %s.\n"
                , PROCESS_NAME
                , mode
            );
        
        }
    
    } else {
    
        if (streq(mode, "factory-param"))
        {
            uint32_t i = Params::get("A_BT_CONNECT_TO");
            if (i < n_factory_addresses)
            {
                daemon_mode = Mode::ONE_CONNECT;
                connect_address = factory_addresses[i];
                valid_connect_address = true;
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
                valid_connect_address = true;
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
    }

	dbg("%s daemon mode %u.\n", PROCESS_NAME, daemon_mode);

	if (daemon_mode == Mode::UNDEFINED)
		return false;


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
