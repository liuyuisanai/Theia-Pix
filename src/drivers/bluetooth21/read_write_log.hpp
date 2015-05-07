#pragma once

#include <unistd.h>

#include <cstdint>
#include <cstring>

#include "repr.hpp"
#include "time.hpp"

struct DevLog {
	const int dev;
	const int log;

	const char * const prefix_read;
	const size_t prefix_read_len;

	const char * const prefix_write;
	const size_t prefix_write_len;

	BT::Time::stamp_t stamp;

	DevLog(int d, int l, const char * prefix_r, const char * prefix_w)
	: dev(d)
	, log(l)
	, prefix_read(prefix_r)
	, prefix_read_len(strlen(prefix_r))
	, prefix_write(prefix_w)
	, prefix_write_len(strlen(prefix_w))
	, stamp(0)
	{}

	friend inline int
	fileno(const DevLog & self) { return self.dev; }

	friend ssize_t
	read(DevLog &self, void *buf, size_t buf_size)
	{
		const ssize_t r = ::read(self.dev, buf, buf_size);
		if (r > 0) { log_data(self, true, buf, r); }
		return r;
	}

	friend ssize_t
	write(DevLog &self, const void *buf, size_t buf_size)
	{
		const ssize_t r = ::write(self.dev, buf, buf_size);
		if (r > 0) { log_data(self, false, buf, r); }
		return r;
	}

	friend void
	write_log(DevLog &self, bool is_read, const void *buf, size_t buf_size)
	{
		int save_errno = errno;

		write_stamp(self);
		if (is_read)
			write(self.log, self.prefix_read, self.prefix_read_len);
		else
			write(self.log, self.prefix_write, self.prefix_write_len);
		write_repr(self.log, buf, buf_size);
		const char ch = '\n';
		write(self.log, &ch, 1);

		errno = save_errno;
	}

	friend void
	write_stamp(DevLog &self)
	{
		auto now = BT::Time::now();
		char buf[24];

		// Use only lower half of useconds as debug session
		// is unlikely to be several hours long.
		snprintf(buf, sizeof buf, "%10u %6u+ "
			, unsigned(now)
			, unsigned(self.stamp == 0 ? 0 : now - self.stamp)
		);
		self.stamp = now;
		write(self.log, buf, strlen(buf));
	}

	friend inline int
	fsync(DevLog & self)
	{
		::fsync(self.log);
		return ::fsync(self.dev);
	}
};
