#include <nuttx/config.h>

#include <fcntl.h>
#include <stdlib.h>

#include <cstdio>
#include <cstring>
#include <ctime>

extern "C" __EXPORT int
main(int argc, const char * const * argv);

#include "at.hpp"

namespace bt_cfg {

#include "read_write_log.hpp"
#include "unique_file.hpp"

#ifdef FORCE_SERIAL_TERMIOS_RAW
# include "serial_config.hpp"
#endif

using namespace std;

int
open_serial_default(const char name[]) {
	int fd = ::open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (fd == -1) {
		perror("open");
		exit(1);
	}

#ifdef FORCE_SERIAL_TERMIOS_RAW
	if (not (serial_set_raw(fd) and serial_set_speed(fd, B57600))) {
		fprintf(stderr, "failed setting raw mode and speed\n");
		exit(1);
	}
#endif

	return fd;
}

template <typename Device>
inline void
read_junk(Device &f) {
	char buf[32];
	ssize_t s;
	do {
		s = ::read(f, s, sizeof(s));
	} while (s > 0);
}

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

} // end of namespace bt_cfg

int
main(int argc, char const * const * argv)
{
	using namespace bt_cfg;
	using namespace std;

	if (argc < 3) {
		fprintf(stderr,"Usage: %s tty commands\n", argv[0]);
		return 1;
	}

	unique_file d = open_serial_default(argv[1]);

	DevLog f { d.get(), 2 };

	bool ok;
	const char * const *p;
	for (p = argv + 2; *p; ++p) {
		const char *cmd = *p;

		if (streq(cmd, "+at"))
			ok = AT::switch_to_at_mode(f);
		else if (streq(cmd, "-at"))
			ok = AT::switch_to_data_mode(f);
		else if (streq(cmd, "reset"))
			ok = AT::exec_reset(f);
		else
			ok = AT::exec_cmd(f, cmd);

		if (not ok) {
			fprintf(stderr, "Failed at command '%s'.\n", cmd);
			return 1;
		}
	}

	return 0;
}
