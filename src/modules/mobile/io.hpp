#pragma once

#include <fcntl.h>

inline bool
switch_nonblock(int fd, bool nonblock)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (nonblock) { flags |= O_NONBLOCK; }
	else { flags &= ~O_NONBLOCK; }
	return fcntl(fd, F_SETFL, flags) != -1;
}

inline bool
set_blocking_mode(int fd) { return switch_nonblock(fd, false); }

inline bool
set_nonblocking_mode(int fd) { return switch_nonblock(fd, true); }
