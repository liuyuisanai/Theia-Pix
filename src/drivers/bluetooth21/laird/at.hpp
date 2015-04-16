#pragma once

#include <termios.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

#include "../buffer_rx.hpp"
#include "../time.hpp"

#include "../std_algo.hpp"

#include "service_params.hpp"
#include "uart.hpp"

namespace BT
{
namespace Service
{
namespace Laird
{

namespace AT
{

constexpr size_t MAX_COMMAND_LEN = 16;

constexpr unsigned TIMEOUT_us = 1000000; /* one second */
constexpr unsigned WAIT_us = 33333; /* 1/30 of a second */

}
// end of namespace AT

template <typename Device>
bool
at_send_cmd(Device & dev, const char cmd[])
{
	const size_t cmd_len = strlen(cmd);
	XtBuffer< AT::MAX_COMMAND_LEN  > buf;
	D_ASSERT(cmd_len + 1 < capacity(buf));

	insert_end_n_unsafe(buf, cmd, cmd_len);
	insert_end_n(buf, 1, '\r');

	ssize_t r = write(dev, buf);
	bool ok = r >= 0 or errno == EAGAIN;
	while (ok and not empty(buf))
	{
		usleep(AT::WAIT_us);
		r = write(dev, buf);
		ok = r >= 0 or errno == EAGAIN;
	}
	if (not ok) { perror("at_send_cmd / write"); }

	return ok;
}

template <typename Device>
bool
at_wait_ok(Device & dev, size_t echo_size)
{
	unsigned timeout = AT::TIMEOUT_us / AT::WAIT_us;

	const char OK_STR[] = "\r\nOK\r\n";
	constexpr size_t OK_LEN = sizeof(OK_STR) - 1;

	/* buf should be "command" + "\r" + "\r\nOK\r\n" + "\0" */
	RxBuffer< AT::MAX_COMMAND_LEN + 1 + OK_LEN + 1 > buf;
	fill(begin(buf), end(buf), 0);

	constexpr size_t waiting = capacity(buf) - 1;

	do
	{
		/*
		 * poll() is overkill in this case
		 * and still requires explicit timeout handling.
		 */
		usleep(AT::WAIT_us);

		ssize_t r = read(dev, buf);
		if (r < 0 and errno != EAGAIN)
		{
			perror("at_wait_ok / read");
			break;
		}

		--timeout;
	}
	while (size(buf) < waiting and timeout > 0);
	if (timeout == 0) { dbg("at_wait_ok timeout reached.\n"); }

	/* read() could have filled buf up to capacity limit. */
	begin(buf)[min(size(buf), capacity(buf) - 1)] = '\0';
	const char * p = strstr((const char*)cbegin(buf), OK_STR);

	return p != nullptr;
}

template <typename Device>
bool
is_at_mode(Device & dev)
{
	bool ok;

	/*
	 * Send single '\r'.
	 * In AT mode, a module replies with either OK or ERROR nn.
	 * In MP mode it should not reply anything.
	 *
	 * It also required as sometimes the module replies an error
	 * becaise of some stuff in uart output buffer.
	 *
	 * In MP mode, one byte should be ignored by the module after a timeout
	 * as minimal packet is 3 bytes.
	 */
	dbg("<- is_at_mode() '\\r'.\n");
	char c = '\r';
	ssize_t r = write(dev, &c, 1);
	if (r == 1)
	{
		unsigned timeout = AT::TIMEOUT_us / AT::WAIT_us;
		do
		{
			usleep(AT::TIMEOUT_us);
			r = read(dev, &c, 1);
			--timeout;
		}
		while (r == 0 and timeout > 0);

		/* read out all the stuff */
		if (r == 1) { at_wait_ok(dev, 0); }
	}
	ok = r == 1;
	if (ok)
	{
		dbg("<- is_at_mode() 'AT\\r'.\n");
		const char cmd[] = "AT";
		at_send_cmd(dev, cmd);
		ok = at_wait_ok(dev, strlen(cmd));
	}
	dbg("is_at_mode() %i.\n", ok);
	return ok;
}

template <typename Device>
bool
at_to_mp_mode(Device & dev)
{
	const char cmd[] = "AT&F*MP*";
	bool ok = at_send_cmd(dev, cmd) and at_wait_ok(dev, strlen(cmd));
	dbg("at_to_mp_mode() %s.\n", ok ? "ok" : "failed");
	return ok;
}

template <typename Device>
bool
at_soft_reset(Device & dev)
{
	const char cmd[] = "ATZ";
	bool ok = at_send_cmd(dev, cmd);

	/* wait until it reboots */
	usleep(MODULE_RESET_WAIT_us);

	/* read out echo, but it has to timeout. */
	at_wait_ok(dev, strlen(cmd));

	dbg("at_soft_reset() %s.\n", ok ? "ok" : "failed");
	return ok;
}

}
// end of namespace Laird
}
// end of namespace Service
}
// end of namespace BT
