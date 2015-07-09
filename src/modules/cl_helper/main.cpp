#include <nuttx/config.h>

#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdlib.h>

#include <cstdio>
#include <cstring>
#include <ctime>

extern "C" __EXPORT int
main(int argc, const char * const * argv);

using namespace std;

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

static void
usage()
{
    fprintf(stderr, "Usage:\n"
        "\tclh pairing [on|off|toggle]\n"
        "\n"
    );
}

#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)
#define PAIRING_TOGGLE      _IOC(_BLUETOOTH21_BASE, 2)

void pairing_on() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_ON, 0);
    }

    close(fd);


}

void pairing_off() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_OFF, 0);
    }

    close(fd);
}


void pairing_toggle() {

    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        ioctl(fd, PAIRING_TOGGLE, 0);
    }

    close(fd);

}

int
main(int argc, char const * const * argv)
{
	if (argc < 3) {
		fprintf(stderr,"Usage: %s tty commands\n", argv[0]);
		return 1;
	}

    if (streq(argv[1], "help")) {
        usage();
    }

    else if (streq(argv[1], "pairing")) {

        if (argc < 3) usage();
        else if (streq(argv[2], "on")) {

            pairing_on();
        
        } else if (streq(argv[2], "off")) {

            pairing_off();

        } else if (streq(argv[2], "toggle")) {

            pairing_toggle();
            
        } else {

            usage();
        }
    
    } else {
        usage();
    }

	return 0;
}
