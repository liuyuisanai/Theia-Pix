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

	BT::Time::stamp_t stamp_read, stamp_write;

	DevLog(int d, int l, const char * prefix_r, const char * prefix_w)
	: dev(d)
	, log(l)
	, prefix_read(prefix_r)
	, prefix_read_len(strlen(prefix_r))
	, prefix_write(prefix_w)
	, prefix_write_len(strlen(prefix_w))
	, stamp_read(0)
	, stamp_write(0)
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

		if (is_read)
		{
			log_stamp(self, self.stamp_read, buf_size);
			log_write(self, self.prefix_read, self.prefix_read_len);
		}
		else
		{
			log_stamp(self, self.stamp_write, buf_size);
			log_write(self, self.prefix_write, self.prefix_write_len);
		}

		write_repr(self.log, buf, buf_size);
		const char ch = '\n';
		log_write(self, &ch, 1);

		errno = save_errno;
	}

	friend void
	log_stamp(DevLog &self, BT::Time::stamp_t & stamp, size_t buf_size)
	{
		auto now = BT::Time::now();
		char buf[36];

		// Use only lower half of useconds as debug session
		// is unlikely to be several hours long.
		snprintf(buf, sizeof buf, "%10u %8udt %4ub "
			, unsigned(now)
			, unsigned(stamp == 0 ? 0 : now - stamp)
			, unsigned(buf_size)
		);
		stamp = now;
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
