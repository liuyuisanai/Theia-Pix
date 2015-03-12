#pragma once

#include "std_algo.hpp"
#include "std_iter.hpp"


namespace BT
{

template <size_t CAPACITY>
struct FIFO
{
	using value_type = uint8_t;
	using iterator = value_type *;
	using const_iterator = const value_type *;

	iterator first, last;
	value_type data[CAPACITY];

	FIFO< CAPACITY >() : first(data), last(data) {}

	FIFO< CAPACITY >(const FIFO< CAPACITY > &) = delete;
	FIFO< CAPACITY > & operator = (const FIFO< CAPACITY > &) = delete;

	iterator begin() { return first; }
	iterator end() { return last; }

	const_iterator begin() const { return first; }
	const_iterator end() const { return last; }
};

template <size_t CAPACITY>
constexpr size_t
capacity(const FIFO< CAPACITY > &) { return CAPACITY; }

template <size_t CAPACITY>
bool
empty(const FIFO< CAPACITY > & self) { return self.first == self.last; }

template <size_t CAPACITY>
size_t
size(const FIFO< CAPACITY > & self) { return self.last - self.first; }

template <size_t CAPACITY>
size_t
space_available(const FIFO< CAPACITY > & self)
{
	return CAPACITY - (self.last - self.data);
}

template <size_t CAPACITY>
bool
full(const FIFO< CAPACITY > & self) { return space_available(self) == 0; }

template <size_t CAPACITY>
void
clear(FIFO< CAPACITY > & self) { self.first = self.last = self.data; }

template <size_t CAPACITY>
void
erase_begin(FIFO< CAPACITY > & self, size_t n)
{
	if (size(self) <= n) { clear(self); }
	else { self.first += n; }
}

template <size_t CAPACITY>
void
pack(FIFO< CAPACITY > & self)
{
	if (self.first != self.data)
	{
		self.last = copy(self.first, self.last, self.data);
		self.first = self.data;
	}
}

template <size_t CAPACITY, typename It>
void
insert_end_unsafe(FIFO< CAPACITY > & self, It first, It last)
{
	self.last = copy(first, last, self.last);
}

template <size_t CAPACITY, typename It>
void
insert_end_n_unsafe(FIFO< CAPACITY > & self, It first, const size_t n)
{
	self.last = copy_n(first, n, self.last);
}

template <size_t CAPACITY, typename T = typename FIFO<CAPACITY>::value_type>
void
insert_end_n(FIFO< CAPACITY > & self, size_t n, const T & x)
{
	n = min(n, space_available(self));
	self.last = fill_n(self.last, n, x);
}

} // end of namespace BT
