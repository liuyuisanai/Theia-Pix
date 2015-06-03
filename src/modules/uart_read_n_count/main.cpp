extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>

#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include <cstdio>

static void
read_n_count(int fd)
{
	pollfd p[2];
	p[0].fd = 0;
	p[0].events = POLLIN;
	p[1].fd = fd;
	p[1].events = POLLIN;

	char buf[512];
	ssize_t r;
	ssize_t total = 0;

	printf("\nReading...\nCtrl+C to stop.\n");

	while (true)
	{
		r = poll(p, 2, -1);
		if (r < 0)
		{
			perror("poll");
			break;
		}

		if (p[0].revents)
		{
			r = read(0, buf, 1);
			if (buf[0] == 0x03) { break; }
		}

		if (p[1].revents)
		{
			r = read(fd, buf, sizeof buf);
			while (r > 0)
			{
				total += r;
				r = read(fd, buf, sizeof buf);
			}
			if (r == 0) { break; }
			if (r < 0 and errno != EAGAIN)
			{
				perror("read");
				break;
			}
		}
	}

	printf("\n\nTotal bytes read: %u\n\n", total);
}

int
main(int argc, const char * const * const argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s tty\n", argv[0]);
		return 1;
	}

	int fd = open(argv[1], O_RDONLY | O_NOCTTY | O_NONBLOCK);

	if (fd < 0)
	{
		perror("open");
		return 1;
	}
	else
	{
		read_n_count(fd);
		close(fd);
		return 0;
	}
}
