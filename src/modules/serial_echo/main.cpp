/****************************************************************************
 *
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
 *   Author: @author Example User <mail@example.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>

#include <cstdio>
#include <cstring>

extern "C" __EXPORT int
main(int argc, char const * const * const argv);

namespace serial_echo {

#include "unique_file.hpp"

#ifdef FORCE_SERIAL_TERMIOS_RAW
# include "serial_config.hpp"
#endif


using namespace std;


int
open_serial_default(const char name[]) {
	int fd = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

#ifdef FORCE_SERIAL_TERMIOS_RAW
	if (not (serial_set_raw(fd) and serial_set_speed(fd, B57600))) {
		fprintf(stderr, "failed setting raw mode and speed\n");
		exit(1);
	}
#endif

	return fd;
}

void
echo_forever(int dev) {
	struct pollfd poll_dev;
	poll_dev.fd = dev;

	static char buf[128];
	ssize_t n = 0;
	ssize_t sr, sw;

	fprintf(stderr, ".");
	fflush(stderr);
	while (true) {
		poll_dev.events = POLLIN;
		if (n > 0) poll_dev.events |= POLLOUT;

		while (poll(&poll_dev, 1, 10000/*ms*/) != 1) {
			fprintf(stderr, ".");
			fflush(stderr);
		}

		sr = 0;
		sw = 0;

		if (poll_dev.revents & POLLIN) {
			sr = read(dev, buf + n, sizeof(buf) - n);
			if (sr > 0) n += sr;
		}

		if (n > 0) {
			sw = write(dev, buf, n);
			if (sw > 0)
				if (sw == n) { n = 0; }
				else
				{
					n -= sw;
					memcpy(buf, buf + sw, n);
				}
		}

		//if (n > 0 or sr > 0 or sw > 0)
		//	fprintf(stderr, "n %d sr %d sw %d\n", n, sr, sw);
	}
}

void
usage(const char * name)
{
	fprintf(stderr,
		"_Forever_ reads data from a char-device and writes it back\n"
		"Usage: %s tty_device\n",
		name);
	exit(1);
}

} // end of namespace serial_echo

int
main(int argc, char const * const * const argv)
{
	using namespace serial_echo;

	if (argc != 2) usage(argv[0]);

	fprintf(stderr, "open(\"%s\")\n", argv[1]); fflush(stderr);
	unique_file dev = open_serial_default(argv[1]);
	fprintf(stderr, "open -> %d\n", dev.get()); fflush(stderr);

	fprintf(stderr, "starting forever loop... bye\n"); fflush(stderr);
	echo_forever(dev.get());

	fprintf(stderr, "quit unexpectedly\n");

	return 0;
}
