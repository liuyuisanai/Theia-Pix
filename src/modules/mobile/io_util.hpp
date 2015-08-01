#pragma once

#include "io_blocking.hpp"

template <typename Device, int TIMEOUT_MS>
ssize_t
read_guaranteed(BlockingDevice<Device, TIMEOUT_MS> & d, void * buf, size_t buf_size)
{
	ssize_t r = read(d, buf, buf_size);
	fprintf(stderr, "read(%u) -> %i.\n", (unsigned)buf_size, (int)r); fflush(stderr);
	return buf_size;
}

template <typename Device>
bool
read_char_guaranteed(Device & d, char & ch)
{ return read_guaranteed(d, &ch, 1); }

template <typename Device>
bool
write_char(Device & d, char ch) {
	bool ok = write(d, &ch, 1) == 1;
	if (not ok) { perror("write_char"); fflush(stderr); }
	return ok;
}

/*
 * The following write() makes it possible to use write_repr() with FILE *.
 */
ssize_t
write(FILE* f, const void * buf, ssize_t buf_size) {
	ssize_t r = fwrite(buf, 1, buf_size, f);
	fflush(f);
	return r;
}
