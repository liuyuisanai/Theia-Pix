#include <nuttx/config.h>

#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdlib.h>

#include <uORB/uORB.h>
#include <uORB/topics/bt_state.h>

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
    printf("Usage:\n"
        "\tclh pairing [on|off|toggle|status]\n"
        "\n"
    );
}

#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)
#define PAIRING_TOGGLE      _IOC(_BLUETOOTH21_BASE, 2)

static int _bt_sub = -1;
static struct bt_state_s _bt_state;

void bt_state() {
	_bt_sub = orb_subscribe(ORB_ID(bt_state));
    if (_bt_sub > 0) {
        orb_copy(ORB_ID(bt_state), _bt_sub, &_bt_state);
        switch (_bt_state.global_state) {
            case INITIALIZING:
                printf("Current bluetooth state: NOT STARTED\n");
                break;
            case PAIRING:
                printf("Current bluetooth state: PAIRING\n");
                break;
            case NO_PAIRED_DEVICES:
                printf("Current bluetooth state: NO_PAIRED_DEVICES\n");
                break;
            case CONNECTING:
                printf("Current bluetooth state: CONNECTING\n");
                break;
            case CONNECTED:
                printf("Current bluetooth state: CONNECTED\n");
                break;
            default:
                printf("Unknown bluetooth state\n");
                break;
        }
    }
}

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
	if (argc < 2) {
        usage();
		return 1;
	}

    if (streq(argv[1], "help")) {
        usage();
    }

    else if (streq(argv[1], "status")) {
        bt_state();
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
