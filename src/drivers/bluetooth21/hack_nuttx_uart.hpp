#pragma once

#include <nuttx/config.h>
#include <cstdint>
#include <sys/ioctl.h>

#include "debug.hpp"

#include "std_algo.hpp"

namespace BT
{
namespace NuttxUART
{

enum class WriteVariant
{
	AS_MUCH_AS_POSSIBLE,
	/*
	 * The following variants are unusable:
	 * - ONE_WHOLE_FITS -- poll() always sees free space for write()
	 *                     that results in 100% cpu usage,
	 * - TOTAL_FREE -- lacks ioctl() call to get TX space used.
	 */
	// ONE_WHOLE_FITS,
	// TOTAL_FREE,
};

template <typename Device, WriteVariant Variant>
struct Hack
{
	Device & d0;
	Hack(Device & dev) : d0(dev) {}
	Hack(Hack<Device, Variant> && h) : d0(h.d0) {}
	Hack(const Hack<Device, Variant> & h) : d0(h.d0) {}

	inline size_t
	available_tx()
	{
		size_t s = 0;
		// NuttX FIONWRITE means free space.
		int r = ioctl(fileno(d0), FIONWRITE, (uintptr_t)&s);
		if (r < 0) { dbg_perror("Hack::available_tx/ioctl"); }
		return s;
	}
};

template <WriteVariant V, typename Device>
inline Hack<Device, V>
make_hack(Device & d) { return Hack<Device, V>(d); }

template <typename D, WriteVariant V>
inline int
fileno(Hack<D, V> & dev) { return fileno(dev.d0); }

template <typename D, WriteVariant V>
inline int
fsync(Hack<D, V> & dev) { return fsync(dev.d0); }

template <typename D, WriteVariant V>
inline ssize_t
read(Hack<D, V> & dev, void * buf, size_t size)
{ return read(dev.d0, buf, size); }

template <typename D>
inline ssize_t
write(Hack<D, WriteVariant::AS_MUCH_AS_POSSIBLE> & dev, const void * buf, size_t size)
{ return write(dev.d0, buf, min(size, dev.available_tx())); }

//template <typename D>
//inline ssize_t
//write(Hack<D, WriteVariant::ONE_WHOLE_FITS> & dev, const void * buf, size_t size)
//{
//	if (size <= dev.available_tx()) { return write(dev.d0, buf, size); }
//
//	errno = EAGAIN;
//	return -1;
//}

//template <typename D>
//inline ssize_t
//write(Hack<D, WriteVariant::TOTAL_FREE> & dev, const void * buf, size_t size)
//{
//	// TODO Add some ioctl().
//	if (dev.available_tx() == CONFIG_USART2_TXBUFSIZE)
//		return write(dev.d0, buf, size);
//
//	errno = EAGAIN;
//	return -1;
//}

}
// end of namespace NuttxUART
}
// end of namespace BT
