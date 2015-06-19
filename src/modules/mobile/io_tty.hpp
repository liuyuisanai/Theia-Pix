#pragma once

#include <fcntl.h>
#include <termios.h>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "debug.hpp"

inline bool
tty_set_speed(int fd, speed_t speed)
{
	struct termios mode;
	bool ok = tcgetattr(fd, &mode) == 0
		and cfsetspeed(&mode, speed) == 0
		and tcsetattr(fd, TCSANOW, &mode) == 0;
	if (not ok)
		perror("tty_set_speed");
	return ok;
}

inline speed_t
tty_get_ispeed(int fd)
{
	struct termios mode;
	if (tcgetattr(fd, &mode) == 0) { return cfgetispeed(&mode); }
	perror("tty_get_ispeed");
	return 0;
}

inline bool
tty_use_ctsrts(int fd)
{
	struct termios mode;
	int r = tcgetattr(fd, &mode);
	if (r == 0)
	{
		mode.c_cflag |= CCTS_OFLOW | CRTS_IFLOW;
		r = tcsetattr(fd, TCSANOW, &mode);
	}
	return r == 0;
}

inline int
tty_open(const char name[])
{
       int r = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);
       if (r < 0) { dbg("tty_open(\"%s\"): %s\n", name, strerror(errno)); }
       return r;
}

