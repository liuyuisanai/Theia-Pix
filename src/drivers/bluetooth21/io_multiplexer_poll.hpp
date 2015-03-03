#pragma once

#include <poll.h>

#include "debug.hpp"
#include "io_multiplexer.hpp"
#include "std_algo.hpp"

namespace BT
{

template <typename Device>
inline void
perform_input(Device & d, MultiPlexer & mp)
{ process_serial_input(mp.protocol_tag, d, mp.rx); }

template <typename Device>
inline void
perform_output(Device & d, MultiPlexer & mp)
{ process_serial_output(d, mp.xt); }

template <typename Device>
inline void
perform_poll_io(Device & d, MultiPlexer & mp, int poll_timeout_ms)
{
	if (empty(mp.xt.device_buffer)) { fill_device_buffer(mp.xt); }

	pollfd p;
	p.fd = fileno(d);
	p.events = POLLIN;
	if (not empty(mp.xt.device_buffer)) { p.events |= POLLOUT; }

	int r = poll(&p, 1, poll_timeout_ms);
	if (r == -1)
		perror("perform_poll_io / poll");
	else if (r == 1)
	{
		/*
		 * Input comes before output as we have
		 * to check what the chip is ready to receive and
		 * to inform the chip about what we are ready to receive.
		 */
		if (p.revents & POLLIN)
			perform_input(d, mp);
		if (p.revents & POLLOUT)
			perform_output(d, mp);
	}
}

}
// end of namespace BT
