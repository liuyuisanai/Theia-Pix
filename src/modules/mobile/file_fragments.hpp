#pragma once

#include <fcntl.h>

#include "std_algo.hpp"

namespace base64 {

struct FragmentReader
{
	size_t bytes_left;
	FragmentReader(size_t size) : bytes_left{ size } {}

	template <typename Device>
	friend inline ssize_t
	read_fragment(FragmentReader & self, Device & d, void * buf, size_t buf_size)
	{
		fprintf(stderr, "read bytes_left %i buf_size %i.\n", self.bytes_left, buf_size); fflush(stderr);
		if (self.bytes_left == 0)
			return 0; /* EoF by read() conventions */
		if (self.bytes_left < buf_size) { buf_size = self.bytes_left; }
		ssize_t s = read(d, buf, buf_size);
		if (s > 0) { self.bytes_left -= s; }
		return s;
	}
};

struct FragmentWriter
{
	size_t bytes_left;
	FragmentWriter(size_t size) : bytes_left{ size } {}

	template <typename Device>
	friend inline ssize_t
	write_fragment(FragmentWriter & self, Device & d, const void * buf, size_t buf_size)
	{
		fprintf(stderr, "write bytes_left %i buf_size %i.\n", self.bytes_left, buf_size); fflush(stderr);
		if (self.bytes_left == 0)
		{
			errno = EFBIG;
			return -1;
		}
		if (self.bytes_left < buf_size) { buf_size = self.bytes_left; }
		ssize_t s = write(d, buf, buf_size);
		if (s > 0) { self.bytes_left -= s; }
		return s;
	}
};

template <int>
struct FragmentOperator;

template <>
struct FragmentOperator< O_RDONLY >
{ using type = FragmentReader; };

template <>
struct FragmentOperator< O_WRONLY >
{ using type = FragmentWriter; };

template <int MODE, typename Device>
struct DeviceFragment
{
	Device & dev;
	typename FragmentOperator< MODE >::type fr;

	DeviceFragment(Device & d, size_t fragment_size)
	: dev(d), fr(fragment_size) {}

	friend int
	fileno(DeviceFragment & self) { return fileno(self.dev); }

	friend inline ssize_t
	read(DeviceFragment & self, void * buf, size_t buf_size)
	{ return read_fragment(self.fr, self.dev, buf, buf_size); }

	friend inline ssize_t
	write(DeviceFragment & self, const void * buf, size_t buf_size)
	{ return write_fragment(self.fr, self.dev, buf, buf_size); }
};

template <int Op, typename Device>
DeviceFragment< Op, Device >
make_fragment(Device & d, size_t fragment_size)
{ return DeviceFragment< Op, Device >(d, fragment_size); }

} // end of namespace base64
