#include <nuttx/config.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "../../io_tty.hpp"
#include "../../module_params.hpp"
#include "../../time.hpp"
#include "../../unique_file.hpp"

#include "../../std_algo.hpp"

static const char * const devname[7] = {
	"/dev/bt1", "/dev/bt2", "/dev/bt3", "/dev/bt4",
	"/dev/bt5", "/dev/bt6", "/dev/bt7"
};

constexpr size_t MAX_PACKET_SIZE = 256;

static int
all_open_close_test()
{
	int i;
	for (i = 0; i < 7; ++i)
	{
		printf("Opening %s\n", devname[i]);

		int fd = open(devname[i], O_RDWR | O_NONBLOCK);
		if (fd == -1)
		{
			perror("all_open_close_test: open");
			break;
		}

		printf("Closing %s\n", devname[i]);

		int r = close(fd);
		if (r == -1)
		{
			perror("all_open_close_test: close");
			break;
		}
	}
	return i == 7 ? 0 : 1;
}

static int
all_open_close_twice_test()
{
	int i;
	for (i = 0; i < 7; ++i)
	{
		printf("Opening %s\n", devname[i]);

		int fd = open(devname[i], O_RDWR | O_NONBLOCK);
		if (fd == -1)
		{
			perror("all_open_close_twice_test: open");
			break;
		}

		printf("Checking if repeated open() fails.\n");

		int fd2 = open(devname[i], O_RDWR | O_NONBLOCK);
		if (fd2 != -1)
		{
			fprintf(stderr,
				"all_open_close_twice_test:"
				" device opened twice\n");
		}

		printf("Closing %s\n", devname[i]);

		int r = close(fd);
		if (r == -1)
		{
			perror("all_open_close_twice_test: close");
			break;
		}

		printf("Checking if repeated close() fails.\n");

		r = close(fd);
		if (r != -1)
		{
			fprintf(stderr,
				"all_open_close_twice_test:"
				" device closed twice\n");
			break;
		}
	}
	return i == 7 ? 0 : 1;
}

static int
tty_open_flow(const char * name)
{
	int fd = BT::tty_open(name);

	if (fd == -1)
	{
		perror("cat_loopback / open");
	}
	// In NuttX all serial terminal names start with "/dev/ttyS".
	else if (strncmp(name, "/dev/ttyS", strlen("/dev/ttyS")) == 0)
	{
		const bool use_ctsrts = BT::Params::get("A_TELEMETRY_FLOW");
		BT::tty_switch_ctsrts(fd, use_ctsrts);
	}

	return fd;
}


static int
cat_loopback(const char * port)
{
	unique_file dev = tty_open_flow(port);

	while (true)
	{
		pollfd p[2];
		p[0].fd = 0;
		p[0].events = POLLIN;
		p[0].revents = 0;
		p[1].fd = fileno(dev);
		p[1].events = POLLIN;
		p[1].revents = 0;

		{
			int r = poll(p, 2, 1000);
			//fprintf(stderr, "poll r %i revents stdin %x dev %x\n",
			//		r, p[0].revents, p[1].revents);
			if (r < 0)
			{
				perror("poll([0, dev])");
				continue;
			}

			fputc(r == 0 ? 'T' : '.', stderr);
			fflush(stderr);
			if (r == 0) { continue; }
		}

		if (p[0].revents)
		{
			char buf[128];
			ssize_t r = read(0, buf, sizeof(buf));
			if (r < 0) { perror("read(0)"); }
			else if (buf[0] == '\x03') { break; }
			else
			{
				r = write(dev, buf, r);
				if (r < 0) { perror("write(dev)"); }
			}
		}
		if (p[1].revents)
		{
			char buf[256];
			ssize_t r = read(dev, buf, sizeof(buf));
			if (r < 0) { perror("read(dev)"); }
			else
			{
				printf("\n~~>");
				fwrite(buf, 1, r, stdout);
				printf("\n");
				fflush(stdout);
			}
		}
	}

	return 0;
}

static int
fixed_load(const char * port, uint32_t freq, uint32_t size)
{
	const int interval_ms = 1000 / freq;
	if (interval_ms == 0) { return 1; }

	if (size == 0 or size > MAX_PACKET_SIZE) { return 1; }

	unique_file dev = tty_open_flow(port);
	if (fileno(dev) == -1) { return 1; }

	pollfd p;
	p.fd = 0;
	p.events = POLLIN;

	uint8_t packet[MAX_PACKET_SIZE];
	uint8_t counter = 0;

	auto stamp = BT::Time::now();
	const useconds_t interval_us = interval_ms * 1000;
	while (true)
	{
		int r;

		r = poll(&p, 1, interval_ms);
		if (r == 1)
		{
			char ch;
			r = read(0, &ch, 1);
			if (r == 1 and ch == 0x03) { break; }
		}

		BT::fill_n(packet, size, counter);
		r = write(dev, packet, size);
		if (r == size)
		{
			auto now = BT::Time::now();
			printf("%7.2fHz\n", 1000000. / (now - stamp));
			stamp = now;
		}
		else
		{
			perror("fixed_load / write");
			usleep(interval_us);
		}

		++counter;

	}

	return 0;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

bool
parse_uint(const char s[], uint32_t &n, const char * & tail)
{
	char *p;
	n = strtoul(s, &p, 10);
	tail = p;
	return tail != s;
}

static void
usage(const char name[])
{
	fprintf(stderr,
		"Usage: %s all-open-close\n"
		"       %s all-open-close-twice\n"
		"       %s loopback tty-like\n"
		"       %s fixed-load tty-like hz bytes\n"
		"\n",
		name, name, name, name
	);
}

#ifdef MODULE_COMMAND
#define XCAT(a, b)  a ## b
#define CONCAT(a, b)  XCAT(a, b)
#define main CONCAT(MODULE_COMMAND, _main)
#endif

extern "C" __EXPORT int
main(int argc, const char * const argv[]);

int
main(int argc, const char * const argv[])
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (argc == 2)
	{
		if (streq(argv[1], "all-open-close"))
			return all_open_close_test();
		else if (streq(argv[1], "all-open-close-twice"))
			return all_open_close_twice_test();
	}
	else if (argc == 3)
	{
		if (streq(argv[1], "loopback"))
			return cat_loopback(argv[2]);
	}
	else if (argc == 5)
	{
		if (streq(argv[1], "fixed-load"))
		{
			uint32_t freq, size;
			const char * tail;
			if (parse_uint(argv[3], freq, tail)
			and parse_uint(argv[4], size, tail)
			) {
				return fixed_load(argv[2], freq, size);
			}
		}
	}

	usage(argv[0]);
	return 1;
}
