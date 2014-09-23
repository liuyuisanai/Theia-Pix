extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <drivers/drv_airleash_kbd.h>
#include <drivers/drv_hrt.h>

namespace {

constexpr int REPORT_INTERVAL_ms = 100;

bool
poll_0_continue()
{
    pollfd p;
    p.fd = 1;
    p.events = POLLIN;
    int r = poll(&p, 1, REPORT_INTERVAL_ms);
    if (r != 1) { return true; }

    char c = 0;
    r = read(0, &c, 1);
    return r == 1 and c != 0x03;
}

void
report_events(pressed_mask_t m[], size_t n)
{
	if (n == 0)
	{
		fprintf(stderr, "No keyboard events.");
		return;
	}

	size_t i = 0;

	printf("%2i ----   0x%04x   ----\n", i, m[i]);

	while(++i < n)
		printf("%2i 0x%04x 0x%04x 0x%04x\n",
			i, m[i-1] & ~m[i], m[i], ~m[i-1] & m[i]);

	printf("%2i ====   0x%04x   ====\n", i, m[i]);
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	int fd = open(KBD_DEVICE_PATH, O_RDONLY | O_NONBLOCK);
	pressed_mask_t buffer[KBD_SCAN_BUFFER_N_ITEMS];
	hrt_abstime stamp = hrt_absolute_time();

	while (poll_0_continue())
	{
		ssize_t s = read(fd, buffer, sizeof(buffer));
		if (s < 0)
		{
			perror(KBD_DEVICE_PATH " read");
			break;
		}

		size_t nr = s / sizeof(*buffer);

		hrt_abstime now = hrt_absolute_time();

		size_t nt = 1 + (now - stamp) / KBD_SCAN_INTERVAL_usec;
		size_t n = nt < nr ? nt : nr; // min(nt, nr)

		printf("\n%08x\n", (int)now);
		report_events(buffer, n);

		stamp = now;
	}

	return 0;
}
