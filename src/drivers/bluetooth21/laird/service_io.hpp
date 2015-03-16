#pragma once

#include <poll.h>

#include <cerrno>
#include <cstdint>
//#include <cstdlib>

#include "../debug.hpp"

#include "service_state.hpp"

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
read_packet(Device & dev, void * buf, size_t size)
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
	return read(dev, buf, size);
}

template <typename Device>
ssize_t
wait_service_packet(Device & dev, RESPONSE_EVENT_UNION & buf)
{
	ssize_t r = read_packet(dev, &buf, sizeof buf);
	if (r == -1 and errno == EAGAIN)
	{
		pollfd p;
		p.fd = fileno(dev);
		p.events = POLLIN;

		r = poll(&p, 1, READ_WAIT_POLL_ms);
		if (r == 1)
			r = read_packet(dev, &buf, sizeof buf);
		else // errno is either set by poll() or is EAGAIN by read().
			r = -1;
	}
	return r;
}

template <typename Device>
bool
wait_command_response(Device & dev, ServiceState & svc, event_id_t cmd, void * buf, size_t size)
{
	// TODO add timeout parameter
	auto time_limit = Time::now() + MAX_COMMAND_DURATION;
	RESPONSE_EVENT_UNION packet;

	while (true)
	{
		ssize_t r = wait_service_packet(dev, packet);
		if (r < 0)
		{
			if (errno != EAGAIN)
			{
				perror("wait_command_response");
				return false;
			}
		}
		else
		{
			size_t read_size = r;
			process_service_packet(svc, packet);
			if (read_size == size and cmd == get_event_id(packet))
			{
				memcpy(buf, &packet, size);
				return true;
			}
		}

		if (time_limit < Time::now())
		{
			dbg("wait_command_response timeout.\n");
			return false;
		}
	}
}

template <typename Device>
struct ServiceBlockingIO
{
	Device & dev;
	ServiceState & svc;

	ServiceBlockingIO(Device & d, ServiceState & s)
	: dev(d), svc(s)
	{}
};

template <typename Device, typename PacketPOD, typename ResponcePOD>
bool
send_receive(
	ServiceBlockingIO< Device > & self
	, PacketPOD & p
	, ResponcePOD & r
) {
	event_id_t cmd = get_event_id(p);
	auto & s = self;
	return write_command(s.dev, &p, sizeof p)
		and wait_command_response(s.dev, s.svc, cmd, &r, sizeof r);
}

template <typename Device, typename PacketPOD, typename ResponcePOD>
bool
send_receive_verbose(
	ServiceBlockingIO< Device > & self
	, PacketPOD & p
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

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
