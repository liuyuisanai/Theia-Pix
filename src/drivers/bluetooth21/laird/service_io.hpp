#pragma once

#include <poll.h>

#include <cerrno>
#include <cstdint>
//#include <cstdlib>

#include "../debug.hpp"
#include "../buffer_rx.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

/*
 * MAX_COMMAND_DURATION is set to 3 seconds ad arbitrium.
 *
 * I suppose the module should process commands much faster.
 * But in some cases it replies really slow.
 * FIXME should we set MAX_COMMAND_DURATION low and reset the module
 * 	 on timeout?
 */
constexpr auto MAX_COMMAND_DURATION = Time::duration_sec(3);

constexpr int READ_WAIT_POLL_ms = 1000;
constexpr int WRITE_SINGLE_POLL_ms = 1000;

using ResponceEventBuffer = RxBuffer< 256 >;

uint8_t
get_channel_id(const ResponceEventBuffer & buf) { return *next(cbegin(buf)); }

uint8_t
get_event_id(const ResponceEventBuffer & buf) { return *next(cbegin(buf), 2); }

const RESPONSE_EVENT_UNION &
as_packet(const ResponceEventBuffer & buf)
{ return *(const RESPONSE_EVENT_UNION *)cbegin(buf); }

template <typename State>
void
process_service_packet(State & s, const ResponceEventBuffer & buf)
{
	uint8_t ch = get_channel_id(buf);
	bool processed;

	if (ch == 0)
	{
		const auto & packet = as_packet(buf);
		processed = handle_service_packet(s, packet);
		if (not processed)
		{
			auto evt = get_event_id(packet);
			if (not is_command(get_event_id(packet)))
			{
				dbg("-> Event 0x%02x dropped.\n", evt);
				dbg_dump("   Event bytes", buf);
			}
			else if (get_response_status(packet) == MPSTATUS_OK)
				dbg("-> CMD 0x%02x OK\n", evt);
			else
				dbg("-> CMD 0x%02x ERROR 0x%02x\n"
					, evt
					, get_response_status(packet)
				);
		}
	}
	else
	{
		dbg("Dropped packet at channel 0x%02x.\n", ch);
		dbg_dump("Dropped packet bytes", buf);
	}
}


template <typename Device>
ssize_t
write_retry_once(Device & dev, const void * packet, size_t size)
{
	ssize_t r = write(dev, packet, size);
	if (r == -1 and errno == EAGAIN)
	{
		pollfd p;
		p.fd = fileno(dev);
		p.events = POLLOUT;

		r = poll(&p, 1, WRITE_SINGLE_POLL_ms);

		if (r == 1) { r = write(dev, packet, size); }
		else if (r == 0)
		{
			errno = EAGAIN;
			r = -1;
		}
	}
	return r;
}

template <typename Device>
bool
write_command(Device & dev, const void * packet, size_t size)
{
	D_ASSERT(((const uint8_t*)packet)[0] == size);
	D_ASSERT(((const uint8_t*)packet)[1] == 0);
	D_ASSERT(is_command(((const uint8_t*)packet)[2]));

	/*
	 * Assume there could not be partial write.
	 */
	ssize_t r = write_retry_once(dev, packet, size);
	if (r == -1 and errno != EAGAIN)
		perror("write_command");
	return r != -1;
}


template <typename Device>
ssize_t
read_packet(Device & dev, ResponceEventBuffer & buf)
{
	/*
	 * Assume
	 * 	read() always return one packet
	 * 	and the buffer is always big enough for it.
	 *
	 * If for any reason assumption is wrong
	 * and receiving a packet requires several reads,
	 * it should be processed by wait_service_packet()
	 * as it knows service state.
	 */
	return read(dev, buf);
}

template <typename Device>
ssize_t
wait_service_packet(Device & dev, ResponceEventBuffer & buf)
{
	ssize_t r = read_packet(dev, buf);
	if (r == -1 and errno == EAGAIN)
	{
		pollfd p;
		p.fd = fileno(dev);
		p.events = POLLIN;

		r = poll(&p, 1, READ_WAIT_POLL_ms);
		if (r == 1)
			r = read_packet(dev, buf);
		else if (r == 0)
		{
			r = -1;
			errno = EAGAIN;
		}
	}
	return r;
}

template <typename Device, typename State>
bool
wait_command_response(Device & dev, State & state, event_id_t cmd, void * buf, size_t bufsize)
{
	// TODO add timeout parameter
	auto time_limit = Time::now() + MAX_COMMAND_DURATION;
	ResponceEventBuffer packet;

	while (true)
	{
		ssize_t r = wait_service_packet(dev, packet);
		if (r < 0)
		{
			if (errno != EAGAIN)
			{
				dbg_perror("wait_command_response");
				return false;
			}
		}
		else
		{
			size_t read_size = r;
			process_service_packet(state, packet);
			if (read_size == bufsize and cmd == get_event_id(packet))
			{
				copy_n(cbegin(packet), bufsize, (uint8_t*)buf);
				return true;
			}
			else if (is_command(get_event_id(packet)))
			{
				dbg("Unexpected command response 0x%02x.",
					get_event_id(packet));
			}
		}

		if (time_limit < Time::now())
		{
			dbg("wait_command_response timeout.\n");
			return false;
		}

		clear(packet);
	}
}

template <typename Device, typename State>
void
wait_process_event(Device & dev, State & state)
{
	ResponceEventBuffer packet;

	ssize_t r = wait_service_packet(dev, packet);
	if (r < 0)
	{
		if (errno != EAGAIN) { dbg_perror("wait_process_event"); }
	}
	else
	{
		process_service_packet(state, packet);
		event_id_t event = get_event_id(packet);
		if (is_command(event))
			dbg("Unexpected command response 0x%02x.\n", event);
	}
}

template <typename Device, typename State>
struct ServiceBlockingIO
{
	Device & dev;
	State & state;

	ServiceBlockingIO(Device & d, State & s) : dev(d), state(s) {}
};

template <typename Device, typename State>
inline ServiceBlockingIO< Device, State >
make_service_io(Device & d, State & s)
{ return ServiceBlockingIO< Device, State >(d, s); }

template <typename Device, typename State, typename PacketPOD>
bool
send(ServiceBlockingIO< Device, State > & self , const PacketPOD & p) {
	bool ok = write_command(self.dev, &p, sizeof p);
	if (not ok) { dbg_perror("send / write_command"); }
	return ok;
}

template <typename Device, typename State, typename PacketPOD, typename ResponcePOD>
bool
send_receive(
	ServiceBlockingIO< Device, State > & self
	, const PacketPOD & p
	, ResponcePOD & r
) {
	if (not write_command(self.dev, &p, sizeof p))
	{
		dbg_perror("send_receive / write_command");
		return false;
	}

	event_id_t cmd = get_event_id(p);
	return wait_command_response(self.dev, self.state, cmd, &r, sizeof r);
}

template <typename Device, typename State, typename PacketPOD, typename ResponcePOD>
bool
send_receive_verbose(
	ServiceBlockingIO< Device, State > & self
	, const PacketPOD & p
	, ResponcePOD & r
) {
	event_id_t cmd = get_event_id(p);
	dbg("<- Command 0x%02x sent.\n", cmd);
	bool ok = send_receive(self, p, r);
	if (ok)
	{
		auto status = get_response_status(r);
		if (status == MPSTATUS_OK)
			dbg("-> Response 0x%02x success.\n", cmd);
		else
			dbg("-> Response 0x%02x error 0x%02x.\n", cmd, status);
	}
	else { dbg("-> Response 0x%02x timeout.\n", cmd); }
	return ok;
}

template <typename Device, typename State>
void
wait_process_event(ServiceBlockingIO< Device, State > & self)
{ wait_process_event(self.dev, self.state); }

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
