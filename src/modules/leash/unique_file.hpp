#pragma once

#include <unistd.h>

namespace {

class unique_file {
private:
	int fd;

public:
	inline
	unique_file() : fd(-1) {}

	inline
	unique_file(int f) : fd(f) {}

	inline
	unique_file(unique_file && other) : fd(other.fd) {
		other.fd = -1;
	}

	inline
	~unique_file() {
		if (fd > -1) close(fd);
	}

	inline
	int get() const { return fd; }

	friend inline ssize_t
	read(const unique_file & uf, void * buf, size_t buf_size)
	{ return ::read(uf.fd, buf, buf_size); }

	friend inline ssize_t
	write(const unique_file & uf, const void * buf, size_t buf_size)
	{ return ::write(uf.fd, buf, buf_size); }
};

}
