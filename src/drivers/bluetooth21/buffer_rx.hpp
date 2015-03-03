#pragma once

#include <cstdint>

#include "fifo.hpp"


namespace BT
{

template <size_t CAPACITY>
struct RxBuffer : public FIFO< CAPACITY >
{
	// TODO check 16 is enough, compare to minimal/maximal packet size
	// TODO make 16 configurable
	static constexpr size_t
	//OVERFLOW_BOUND = CAPACITY - std::min<size_t>(16, CAPACITY / 4);
	OVERFLOW_BOUND = CAPACITY - (CAPACITY < 16 ? CAPACITY / 4 : 16);
};

template <size_t CAPACITY>
bool
overflow_expected( RxBuffer< CAPACITY > & b)
{
	return size(b) >= b.OVERFLOW_BOUND;
}

} // end of namespace BT

/*
 * Namespace note: read() declared outside namespace to make it able
 * to use underlying read() call.
 */
template <typename Device, size_t CAPACITY>
ssize_t
read(Device & d, BT::RxBuffer< CAPACITY > & b)
{
	// TODO get rid of local_buf
	using char_type = typename BT::RxBuffer< CAPACITY >::value_type;
	char_type local_buf[ CAPACITY ];

	ssize_t r = read(d, local_buf, space_available(b));
	if (r > 0) { insert_end_unsafe(b, local_buf, local_buf + r); }

	return r;
}
