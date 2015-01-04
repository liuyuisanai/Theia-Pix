#pragma once

#include <cstdio>
#include <unistd.h>

class unique_file {
private:
	int fd;

public:
	inline
	unique_file() : fd(-1) {}

	inline
	unique_file(int x) : fd(x) {}

	inline
	unique_file(unique_file && other) : fd(other.fd) { other.fd = -1; }

	inline
	~unique_file() { if (fd > -1) { close(fd); } }

	inline int
	get() { return fd; }

	friend inline int
	fileno(unique_file & uf) { return uf.fd; }

	friend inline std::size_t
	read(unique_file & uf, void * buf, size_t buf_size)
	{ return ::read(uf.fd, buf, buf_size); }

	friend inline std::size_t
	write(unique_file & uf, const void * buf, size_t buf_size)
	{ return ::write(uf.fd, buf, buf_size); }
};
