#include <nuttx/config.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <cstdio>
#include <unistd.h>

#include <drivers/drv_tone_alarm.h>
#include <drivers/drv_hrt.h>

#include <uORB/topics/bt_state.h>

#include "daemon.hpp"
#include "device_connection_map.hpp"
#include "factory_addresses.hpp"
#include "io_multiplexer_flags.hpp"
#include "io_multiplexer_global.hpp"
#include "svc_globals.hpp"
#include "io_tty.hpp"
#include "laird/configure.hpp"
#include "laird/commands.hpp"
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

int _bt_state_pub = -1;

static volatile bool
should_run = false;

static volatile bool
running = false;

static volatile bool
started = false;

enum class ConnectMode : uint8_t
{
	UNDEFINED,
	LISTEN,
	ONE_CONNECT,
};

enum class ServiceMode : uint8_t
{
	UNDEFINED,
	FACTORY,
	USER,
};

static volatile ConnectMode
connect_mode = ConnectMode::UNDEFINED;

ServiceMode
service_mode = ServiceMode::UNDEFINED;

bool 
valid_connect_address = false;

static Address6
connect_address;

static Address6
pairing_address;

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

template <typename ServiceState>
static void
publish_bt_state(ServiceState & svc) {

    bt_state_s bt_state;
    bt_state.global_state = svc.global_state;

    dbg("Publishing bt_state: %d\n", bt_state);


	if (_bt_state_pub > 0) {
		orb_publish(ORB_ID(bt_state), _bt_state_pub, &bt_state);

	} else {
		_bt_state_pub = orb_advertise(ORB_ID(bt_state), &bt_state);
	}

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

    svc.pairing.paired_devices = 0;
    svc.pairing.pairing_active = true;

    if (connect_mode == ConnectMode::LISTEN) {

        switch_discoverable(service_io, true);

        svc.pairing.pairing_initiator = false;

        while (svc.pairing.pairing_active){

            paired = pair(service_io);
            svc.pairing.pairing_active = check_pairing_enabled();
        }

        switch_discoverable(service_io, false);

    } else if  (connect_mode == ConnectMode::ONE_CONNECT) {

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

            const auto wait_for = Time::duration_sec(10);
            auto time_limit = Time::now() + wait_for;

            while (svc.pairing.paired_devices == 0) {
                sleep(100); 
                if (time_limit < Time::now())
                    break;
            }

            paired = svc.pairing.paired_devices > 0;

            Globals::Service::turn_pairing_off();

            if (paired) {
                connect_address = pairing_address;
                valid_connect_address = true;
            } 
        } 

        if (!paired) {
            dbg("Paired failed.\n");
            // fail actions
        } else {

            dbg("Paired with %d devices ! \n", svc.pairing.paired_devices );
        
        }

    }

}

template <typename ServiceIO, typename ServiceState>
static void
synced_loop(MultiPlexer & mp, ServiceIO & service_io, ServiceState & svc)
{
	using namespace Laird;

	auto & dev = service_io.dev;
	int8_t rssi = -128;
	uint8_t link_quality = 0;

	while (should_run and not module_rebooted(svc.sync))
	{

		wait_process_event(service_io);
		set_xt_ready_mask(mp, svc.flow.xt_mask);

        if (service_mode == ServiceMode::USER) {
        // In user mode we can do pairing
        // Check every 1 scond if pairing is enabled

            if (hrt_elapsed_time(&last_pairing_check) > 1000000){

                last_pairing_check = hrt_absolute_time();

                if (check_pairing_enabled()) {
                    if (!pairing_done_this_cycle) {

                        dbg("Pairing activated\n");

                        svc.global_state = GLOBAL_BT_STATE::PAIRING;

                        publish_bt_state(svc);

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

        if (count_connections(svc.conn) > 0) {

            svc.global_state = GLOBAL_BT_STATE::CONNECTED;

        } else if (svc.pairing.paired_devices == 0) {

            svc.global_state = GLOBAL_BT_STATE::NO_PAIRED_DEVICES;

        } else {

            svc.global_state = GLOBAL_BT_STATE::CONNECTING;

        }

        publish_bt_state(svc);

		dbg("Connections waiting %i, total count %u.\n"
			, no_single_connection(svc.conn)
			, count_connections(svc.conn)
		);

		if (no_single_connection(svc.conn))
		{
			if (connect_mode == ConnectMode::ONE_CONNECT
			and allowed_connection_request(svc.conn)
            and valid_connect_address
			) {
				request_connect(dev, svc.conn, connect_address);
				dbg("Request connect.\n");
			}
		}
		else
		{
			if (connect_mode == ConnectMode::ONE_CONNECT)
			{
				dbg("Requesting RSSI and link quality.\n");
				if (request_rssi_linkquality(service_io, connect_address, rssi, link_quality)) {
					dbg("Link RSSI is %i, quality is %u.\n", rssi, link_quality);
					log_link_quality(svc.sdlog, link_quality, rssi);
				}
			}
		}

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
	should_run = (service_mode != ServiceMode::UNDEFINED 
        and connect_mode != ConnectMode::UNDEFINED
		and fileno(raw_dev) > -1
		and sync_soft_reset(service_io, svc.sync)
		and configure_before_reboot(service_io, (uint8_t)service_mode)
		and sync_soft_reset(service_io, svc.sync)
		and configure_after_reboot(service_io)
		and dump_s_registers(service_io)
	);

    if (should_run && service_mode == ServiceMode::FACTORY) {

        Globals::Service::turn_pairing_on();
		should_run = configure_factory(service_io);

        if (should_run && connect_mode == ConnectMode::LISTEN)
            should_run = switch_discoverable(service_io, true);

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

    service_io.state.pairing.paired_devices = trusted_db_record_count_get(service_io, 1);

    if (connect_mode == ConnectMode::ONE_CONNECT) {

        if (service_mode == ServiceMode::FACTORY) {
        
            int32_t connect_to_param = Params::get("A_BT_CONNECT_TO");

            if (connect_to_param >= 0 && connect_to_param < (int32_t)n_factory_addresses)
            {
                connect_address = factory_addresses[connect_to_param];
                valid_connect_address = true;

            } else {

                fsync(service_io.dev);
                inquiry_loop(service_io, svc.inq);
                should_run = should_run and closest_is_obvious(svc.inq);
                if (should_run) {
                    connect_address = closest_address(svc.inq);
                    valid_connect_address = true;
                }
            }

        }

        if (service_mode == ServiceMode::USER) {

            // Get connect address form trust_db if we paired before.
            if (service_io.state.pairing.paired_devices > 0) {
                connect_address = get_trusted_address(service_io, 1, 1);
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
start(const char cmode[], const char smode[])
{
	dbg("%s Service::start(%s, %s).\n", PROCESS_NAME, smode, cmode);

	connect_mode = ConnectMode::UNDEFINED;
	service_mode = ServiceMode::UNDEFINED;

    uint32_t service_mode_param = Params::get("A_BT_SVC_MODE");

    if (service_mode_param == 0) service_mode = ServiceMode::FACTORY;
    if (service_mode_param == 1) service_mode = ServiceMode::USER;


    // If smode argumet is used service_mode will be overwritten
    if (streq(smode, "factory")) {
        service_mode = ServiceMode::FACTORY;
    } else if (streq(smode, "user")) {
        service_mode = ServiceMode::USER;
    }

    if (streq(cmode, "listen")) {
        connect_mode = ConnectMode::LISTEN;
    } else if (streq(cmode, "one-connect")) {
        connect_mode = ConnectMode::ONE_CONNECT;
    }

	dbg("%s service mode %u.\n", PROCESS_NAME, service_mode);
	dbg("%s connect mode %u.\n", PROCESS_NAME, connect_mode);

	if (service_mode == ServiceMode::UNDEFINED)
		return false;

	if (connect_mode == ConnectMode::UNDEFINED)
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
