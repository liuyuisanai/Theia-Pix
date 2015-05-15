#pragma once

#include <unistd.h>

#include <cstdio>

#include "debug.hpp"
#include "std_util.hpp"

namespace {
using BT::swap;

inline int
fileno(int fd) { return fd; }

class unique_file {
private:
	int fd;

public:
	inline
	unique_file() : fd(-1) {}

	inline
	unique_file(int x) : fd(x) { dbg("unique_file(%i).\n", fd); }

	inline
	unique_file(unique_file && other) : fd(other.fd) { other.fd = -1; }

	friend inline void
	close(unique_file & self)
	{
		if (self.fd > -1)
		{
			dbg("unique_file close(%i).\n", self.fd);
			close(self.fd);
			self.fd = -1;
		}
	}

	inline
	~unique_file() { close(*this); }

	inline unique_file &
	operator = (unique_file && other)
	{
		swap(fd, other.fd);
		return *this;
	}

	inline int
	get() { return fd; }

	inline void
	set(int x)
	{
		close(*this);
		fd = x;
		dbg("unique_file set (%i).\n", fd);
	}

	friend inline int
	fileno(const unique_file & uf) { return uf.fd; }

	friend inline std::size_t
	read(unique_file & uf, void * buf, size_t buf_size)
	{ return read(uf.fd, buf, buf_size); }

	friend inline std::size_t
	write(unique_file & uf, const void * buf, size_t buf_size)
	{ return write(uf.fd, buf, buf_size); }

	friend inline int
	fsync(unique_file & uf) { return ::fsync(uf.fd); }
};

class unique_FILE {
private:
	FILE * fp;

public:
	inline
	unique_FILE() : fp(nullptr) {}

	inline
	unique_FILE(FILE * x)
	: fp(x)
	{ dbg("unique_FILE(%i @ %p).\n", (fp ? fileno(fp) : -1), fp); }

	inline
	unique_FILE(unique_FILE && other) : fp(other.fp) { other.fp = nullptr; }

	friend inline void
	close(unique_FILE & self)
	{
		if (self.fp != nullptr)
		{
			dbg("unique_FILE close(%i @ %p).\n",
				fileno(self.fp), self.fp);
			fclose(self.fp);
			self.fp = nullptr;
		}
	}

	inline
	~unique_FILE() { close(*this); }

	inline unique_FILE &
	operator = (unique_FILE && other)
	{
		swap(fp, other.fp);
		return *this;
	}

	inline FILE *
	get() { return fp; }

	inline void
	set(FILE * x)
	{
		close(*this);
		fp = x;
		dbg("unique_FILE set(%i @ %p).\n", (fp ? fileno(fp) : -1), fp);
	}

	friend inline int
	fileno(unique_FILE const & uf) { return fileno(uf.fp); }

	friend inline std::size_t
	read(unique_FILE & uf, void * buf, size_t buf_size)
	{ return fread(buf, 1, buf_size, uf.fp); }

	friend inline std::size_t
	write(unique_FILE & uf, const void * buf, size_t buf_size)
	{ return fwrite(buf, 1, buf_size, uf.fp); }

	friend inline int
	fsync(unique_FILE & uf) { return ::fsync(fileno(uf.fp)); }
};

}
// end of anonymous namespace
