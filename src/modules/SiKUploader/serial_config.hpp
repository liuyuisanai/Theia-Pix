#pragma once

#include <termios.h>

void
termios_raw_mode(struct termios & mode) {
	mode.c_iflag = 0;
	mode.c_oflag = 0;
	mode.c_lflag = 0;

	mode.c_cflag &= ~CSIZE;
	mode.c_cflag |= CS8;
	mode.c_cflag &= ~CSTOPB;
	mode.c_cflag &= ~(PARENB | PARODD);
	mode.c_cflag |= CREAD;
	mode.c_cflag |= CLOCAL;
}

bool
serial_set_raw(int fd) {
	struct termios mode;
	bool ok = tcgetattr(fd, &mode) == 0;
	if (ok)
	{
		termios_raw_mode(mode);
		ok = tcsetattr(fd, TCSANOW, &mode) == 0;
	}
	return ok;
}

bool
serial_set_speed(int fd, speed_t speed) {
	struct termios mode;
	return tcgetattr(fd, &mode) == 0
		and cfsetspeed(&mode, speed) == 0
		and tcsetattr(fd, TCSANOW, &mode) == 0;
}

speed_t
serial_get_in_speed(int fd) {
	struct termios mode;
	tcgetattr(fd, &mode);
	return cfgetispeed(&mode);
}

speed_t
serial_get_out_speed(int fd) {
	struct termios mode;
	tcgetattr(fd, &mode);
	return cfgetospeed(&mode);
}
