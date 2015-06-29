extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Supported only STM32.
#endif

#include <fcntl.h>
#include <unistd.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <board_config.h>
#include <drivers/drv_adc.h>
#include <uORB/uORB.h>
#include <uORB/topics/system_power.h>

namespace {

inline uint32_t
pin_ref(uint32_t port, uint32_t pin)
{
	return ( (port << GPIO_PORT_SHIFT) & GPIO_PORT_MASK )
		| ( (pin << GPIO_PIN_SHIFT) & GPIO_PIN_MASK );
}

bool
parse_uint(const char s[], uint32_t &n, const char * & tail)
{
	char *p;
	n = std::strtoul(s, &p, 10);
	tail = p;
	return tail != s;
}

bool
parse_port_pin(const char s[], uint32_t & port, uint32_t & pin, const char * & tail)
{
	if (strlen(s) < 3 or toupper(s[0]) != 'P') { return false; }
	char p = toupper(s[1]);
	if (p < 'A' or p >= 'A' + STM32_NGPIO_PORTS) { return false; }
	port = p - 'A';
	uint32_t n;
	if (parse_uint(s + 2, n, tail) and n < 16)
	{
		pin = n;
		return true;
	}
	return false;
}

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

static void
usage(const char * name)
{
	fprintf(stderr, "Usage: %s show pin [...]\n"
			"   or: %s set pin=0 pin=1\n"
			"Pin is 'p' a..g 0..15\n"
			"\n", name, name);
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	if (argc < 3)
	{
		usage(argv[1]);
		return 1;
	}

	if (streq(argv[1], "show"))
	{
		uint32_t mask = 0;
		for (int i=2; i < argc; ++i)
		{
			mask <<= 1;

			uint32_t port, pin;
			const char * tail;
			if (not parse_port_pin(argv[i], port, pin, tail) or *tail != 0)
			{
				fprintf(stderr, "Invalid pin: %s\n", argv[i]);
				continue;
			}
			mask |= stm32_gpioread(pin_ref(port, pin));
		}

		printf("show:");
		uint32_t one = 1 << (argc - 3);
		while (one)
		{
			printf(" %i", (mask & one) != 0);
			one >>= 1;
		}
		printf("\n");
	}
	else if (streq(argv[1], "set")) {
		for (int i=2; i < argc; ++i)
		{
			uint32_t port, pin;
			const char * tail;
			if (not parse_port_pin(argv[i], port, pin, tail))
			{
				fprintf(stderr, "Invalid pin: %s\n", argv[i]);
				continue;
			}
			if (tail[0] == '\0')
			{
				fprintf(stderr, "Use %s=0 or %s=1.\n",
						argv[i], argv[i]);
				continue;
			}
			if (tail[0] != '='
			or not (tail[1] == '0' or tail[1] == '1')
			or tail[2] != '\0')
			{
				fprintf(stderr, "Invalid set %s\n", argv[i]);
				continue;
			}
			printf("%s\n", argv[i]);
			stm32_gpiowrite(pin_ref(port, pin), tail[1] == '1');
		}
	}
	else {
		usage(argv[1]);
		return 1;
	}

	return 0;
}
