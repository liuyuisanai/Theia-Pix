#pragma once

#include <unistd.h>

#include <cstdint>
#include <cstring>

#include "repr.hpp"
#include "time.hpp"

#include "debug.hpp"

namespace BT
{

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
	log_data(DevLog &self, bool is_read, const void *buf, size_t buf_size)
	{
		int save_errno = errno;

		log_stamp(self);
		if (is_read)
			log_write(self, self.prefix_read, self.prefix_read_len);
		else
			log_write(self, self.prefix_write, self.prefix_write_len);
		write_repr(self.log, buf, buf_size);
		const char ch = '\n';
		log_write(self, &ch, 1);

		errno = save_errno;
	}

	friend void
	log_stamp(DevLog &self)
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
		log_write(self, buf, strlen(buf));
	}

	friend size_t
	log_write(DevLog &self, const void * buf, size_t buf_size)
	{
		ssize_t r = ::write(self.log, buf, buf_size);
		if (r < 0) { dbg_perror("DevLog::log_write"); }
		return r;
	}

	friend inline int
	fsync(DevLog & self)
	{
		::fsync(self.log);
		return ::fsync(self.dev);
	}
};

}
// end of namespace BT
