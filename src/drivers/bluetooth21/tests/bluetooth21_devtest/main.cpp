#include <nuttx/config.h>
#include <fcntl.h>

#include <cstdio>
#include <cstring>

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

		printf("Checking repeated open() fails.\n");

		int fd2 = open(devname[i], O_RDWR | O_NONBLOCK);
		if (fd2 != -1)
		{
			fprintf(stderr,
				"all_open_close_twice_test:"
				" device opened twice\n");
			break;
		}

		printf("Closing %s\n", devname[i]);

		int r = close(fd);
		if (r == -1)
		{
			perror("all_open_close_twice_test: close");
			break;
		}

		printf("Checking repeated close() fails.\n");

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
	if (argc != 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "all-open-close"))
		return all_open_close_test();
	else if (streq(argv[1], "all-open-close-twice"))
		return all_open_close_twice_test();

	usage(argv[0]);
	return 1;
}
