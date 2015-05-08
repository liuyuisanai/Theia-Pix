#pragma once

#include <fcntl.h>
#include <termios.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "debug.hpp"

namespace BT
{

inline bool
tty_set_speed(int fd, speed_t speed)
{
	struct termios mode;
	bool ok = tcgetattr(fd, &mode) == 0
		and cfsetspeed(&mode, speed) == 0
		and tcsetattr(fd, TCSANOW, &mode) == 0;
	if (not ok)
		dbg_perror("tty_set_speed");
	return ok;
}

inline speed_t
tty_get_ispeed(int fd)
{
	struct termios mode;
	if (tcgetattr(fd, &mode) == 0) { return cfgetispeed(&mode); }
	dbg_perror("tty_get_ispeed");
	return 0;
}

inline bool
tty_switch_ctsrts(int fd, bool on)
{
	struct termios mode;
	int r = tcgetattr(fd, &mode);
	if (r == 0)
	{
		if (on) { mode.c_cflag |= CCTS_OFLOW | CRTS_IFLOW; }
		else { mode.c_cflag &= ~(CCTS_OFLOW | CRTS_IFLOW); }
		r = tcsetattr(fd, TCSANOW, &mode);
		if (r != 0) { dbg_perror("tcsetattr"); }
	}
	else { dbg_perror("tcgetattr"); }
	dbg("tty_switch_ctsrts(%i, %i) %s.\n"
		, fd
		, on
		, r == 0 ? "ok" : "failed"
	);
	return r == 0;
}

inline int
tty_open(const char name[])
{
       int r = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
       if (r < 0) { dbg("tty_open(\"%s\"): %s\n", name, strerror(errno)); }
       return r;
}

}
// end of namespace BT
