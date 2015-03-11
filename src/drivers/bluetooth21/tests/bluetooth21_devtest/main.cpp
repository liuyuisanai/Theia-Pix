#include <nuttx/config.h>
#include <fcntl.h>
#include <poll.h>

#include <cstdio>
#include <cstring>

#include "../../unique_file.hpp"

static const char * const devname[7] = {
	"/dev/bt1", "/dev/bt2", "/dev/bt3", "/dev/bt4",
	"/dev/bt5", "/dev/bt6", "/dev/bt7"
};

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
cat_loopback(const char * name)
{
	unique_file dev = open(name, O_RDWR);
	if (fileno(dev) == -1)
	{
		perror("cat_loopback / open");
		return 1;
	}

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

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{
	fprintf(stderr, "Usage: %s all-open-close\n", name);
	fprintf(stderr, "       %s all-open-close-twice\n", name);
	fprintf(stderr, "\n");
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
		if (streq(argv[1], "cat-loopback"))
			return cat_loopback(argv[2]);
	}

	usage(argv[0]);
	return 1;
}
