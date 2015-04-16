#pragma once

#include <termios.h>
#include <unistd.h>

#include "../io_tty.hpp"

#include "at.hpp"
#include "defs.hpp"
#include "service_params.hpp"

namespace BT
{
namespace HostProtocol
{

template <typename Device>
bool
setup_serial(LairdProtocol, Device & dev)
{
	using namespace Service::Laird;

	const int fd = fileno(dev);

	bool ok = tty_set_speed(fd, BAUDRATE_AT);
	dbg("AT ispeed %i.\n", tty_get_ispeed(fd));

	if (ok and is_at_mode(dev))
		ok = at_to_mp_mode(dev) and at_soft_reset(dev);
	else
		sleep(1); /* Let device resync by timeout. */

	ok = tty_set_speed(fd, BAUDRATE_MP) and ok;
	dbg("MP ispeed %i.\n", tty_get_ispeed(fd));

	return ok;
}

}
// end of namespace HostProtocol
}
// end of namespace BT
