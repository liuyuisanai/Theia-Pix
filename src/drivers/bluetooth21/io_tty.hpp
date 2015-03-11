#pragma once

#include <fcntl.h>
#include <termios.h>

namespace BT
{

inline bool
tty_set_speed(int fd, speed_t speed)
{
	struct termios mode;
	return tcgetattr(fd, &mode) == 0
		and cfsetspeed(&mode, speed) == 0
		and tcsetattr(fd, TCSANOW, &mode) == 0;
}

inline int
tty_open(const char name[])
{ return open(name, O_RDWR | O_NONBLOCK | O_NOCTTY); }

}
// end of namespace BT
