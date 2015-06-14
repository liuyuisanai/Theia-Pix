extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>

#ifndef CONFIG_ARCH_CHIP_STM32
# error Supported only STM32.
#endif

#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>
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

bool
show(const char *argv) {

    uint32_t port, pin;
    const char * tail;
    if (not parse_port_pin(argv, port, pin, tail) or *tail != 0)
    {
        fprintf(stderr, "Invalid pin: %s\n", argv);
        return false;
    }
    printf("show: %s = %i\n", argv, stm32_gpioread(pin_ref(port, pin)));
    return true;
}

bool
wait(const char *argv) {
    uint32_t n;
    const char * tail;
    if (not parse_uint(argv, n, tail)) {
        fprintf(stderr, "Invalid waiting time %s\n", argv);
        return false;
    }
    usleep(n*1000);
    return true;
}

bool
set(const char *argv) {
    uint32_t port, pin;
    const char * tail;
    if (not parse_port_pin(argv, port, pin, tail))
    {
        fprintf(stderr, "Invalid pin: %s\n", argv);
        return false;
    }
    if (tail[0] == '\0')
    {
        fprintf(stderr, "Use %s=0 or %s=1.\n",
                argv, argv);
        return false;
    }
    if (tail[0] != '='
    or not (tail[1] == '0' or tail[1] == '1')
    or tail[2] != '\0')
    {
        fprintf(stderr, "Invalid set %s\n", argv);
        return false;
    }
    printf("%s\n", argv);
    stm32_gpiowrite(pin_ref(port, pin), tail[1] == '1');
    return true;
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
    char *current_cmd = "NONE";
    bool next = false;
    for (int i = 2; i < argc; i++) {
        if (next) {
            if (streq(current_cmd, "show")) {
                next = show(argv[i]);
            }
            else if (streq(current_cmd, "set")) {
                next = set(argv[i]);
            }
            else if (streq(current_cmd, "wait")) {
                next = wait(argv[i]);
            }
            else {
                if (streq(argv[i], "set")) {
                    current_cmd = "set";
                    next = true;
                }
                else if (streq(argv[i], "show")) {
                    current_cmd = "show";
                    next = true;
                }
                else if (streq(argv[i], "wait")) {
                    current_cmd = "wait";
                    next = true;
                }
                else {
                    usage(argv[1]);
                    return 1;
                }
            }
        }
    }

	return 0;
}
