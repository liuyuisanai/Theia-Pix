#pragma once

#include <sys/types.h>

// #include "repr.hpp"

struct DevLog {
	int dev;
	int log;

	DevLog(int d, int l) : dev {d}, log {l} {}

	friend ssize_t
	read(DevLog &self, void *buf, size_t buf_size)
	{
		ssize_t r = read(self.dev, buf, buf_size);
		if (r > 0) { write_log(self, true, buf, r); }
		return r;
	}

	friend ssize_t
	write(DevLog &self, const void *buf, size_t buf_size)
	{
		ssize_t r = write(self.dev, buf, buf_size);
		if (r > 0) { write_log(self, false, buf, buf_size); }
		return r;
	}

	friend void
	write_log(DevLog &self, bool rw, const void *buf, size_t buf_size)
	{
		write(self.log, buf, buf_size);

		// static const char r[]          = "read   ";
		// static const char w[sizeof(r)] = "write  ";
		//
		// write(self.log, rw ? r : w, sizeof(r) - 1);
		// write_repr(self.log, buf, buf_size);
		// write(self.log, "\n", 2);
	}
};
