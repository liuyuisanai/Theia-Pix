#pragma once

#include <cstdint>

#include "fifo.hpp"

namespace BT
{

template <size_t CAPACITY>
struct XtBuffer : public FIFO< CAPACITY >
{};

} // end of namespace BT

/*
 * Namespace note: write() declared outside namespace to make it able
 * to use underlying write() call.
 */
template <typename Device, size_t CAPACITY>
ssize_t
write(Device & d, BT::XtBuffer< CAPACITY > & b)
{
	ssize_t r = write(d, (void*)begin(b), size(b));
	if (r > 0)
	{
		erase_begin(b, r);
		pack(b);
	}
	return r;
}
