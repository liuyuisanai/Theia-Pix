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
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>


using namespace std;


extern "C" __EXPORT int
main(int argc, char ** const argv);


void
echo_forever(int dev) {
	struct pollfd poll_dev;
	poll_dev.fd = dev;

	char buf[32];
	ssize_t n = 0;
	ssize_t sr, sw;
	while (true) {
		poll_dev.events = POLLIN;
		if (n > 0) poll_dev.events |= POLLOUT;

		while (poll(&poll_dev, 1, 10000/*ms*/) != 1)
			/* wait forever */ ;
		
		sr = 0;
		sw = 0;

		if (poll_dev.revents & POLLIN) {
			sr = read(dev, buf + n, sizeof(buf) - n);
			if (sr > 0) n += sr;
		}

		if (n > 0) {
			sw = write(dev, buf, n);
			if (sw > 0)
       				if (sw == n) n = 0;
				else {
					n -= sw;
					memcpy(buf, buf + sw, n);
				}
		}

		if (n > 0 or sr > 0 or sw > 0)
			fprintf(stderr, "n %d sr %d sw %d\n", n, sr, sw);
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

int
main(int argc, char ** const argv)
{
	if (argc != 2) usage(argv[0]);

	int dev = open(argv[1], O_RDWR | O_NONBLOCK | O_NOCTTY );
	if (dev < 0) {
		perror("open");
		usage(argv[0]);
	}

	echo_forever(dev);
}
