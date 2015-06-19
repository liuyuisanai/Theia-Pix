#pragma once

#include <unistd.h>

template <typename size_type = size_t>
struct buffer_use_levels {
	size_type n_first, n_last;

	buffer_use_levels() : n_first{ 0 }, n_last{ 0 } {}
	buffer_use_levels(buffer_use_levels const &) = default;

	inline buffer_use_levels &
	operator = (buffer_use_levels const & other)
	{
		n_first = other.n_first;
		n_last = other.n_last;
		return *this;
	}

	inline void
	reset()
	{
		n_first = 0;
		n_last = 0;
	}

	inline size_type
	size() const { return n_last - n_first; }

	inline bool
	empty() const { return size() == 0; }

	inline void
	mark_front_removed(size_type n)
	{
		n_first += n;
		if (n_last <= n_first) { reset(); }
	}

	inline void
	mark_back_appended(size_type n) { n_last += n; }

	template <typename It>
	inline It
	first(It base) const { /* std::next */ return base + n_first; }

	template <typename It>
	inline It
	last(It base) const { /* std::next */ return base + n_last; }
};

template <size_t N, typename value_type=char>
struct sink_buffer : protected buffer_use_levels<size_t> /* TODO smaller size_t */ {
	using pointer = value_type *;
	using const_pointer = const value_type *;
	value_type buffer[N];

	sink_buffer() = default;

	using buffer_use_levels<size_t>::reset;
	using buffer_use_levels<size_t>::size;
	using buffer_use_levels<size_t>::empty;

	inline const_pointer
	begin() const { return first(buffer); }

	inline pointer
	begin() { return first(buffer); }

	inline const_pointer
	end() const { return last(buffer); }

	inline pointer
	end() { return last(buffer); }

	inline size_t
	available_tail_size() { return N - n_last; }

	inline bool
	full() const { return N == n_last; }

	inline void
	mark_tail_used(size_t n) { mark_back_appended(n); }

	template <typename Device>
	friend inline ssize_t
	read(Device & d, sink_buffer & self)
	{
		ssize_t s = read(d, self.end(), self.available_tail_size());
		if (s > 0) { self.mark_tail_used(s); }
		return s;
	}

	inline void
	append(value_type const & x) {
		*end() = x;
		mark_back_appended(1);
	}
};

template <size_t N, typename value_type=char>
struct source_buffer : protected buffer_use_levels<size_t> /* TODO smaller size_t */ {
	using pointer = value_type *;
	using const_pointer = const value_type *;
	value_type buffer[N];

	source_buffer() = default;
	source_buffer(source_buffer const &) = default;

	source_buffer &
	operator = (source_buffer const &) = default;

	using buffer_use_levels<size_t>::reset;
	using buffer_use_levels<size_t>::size;
	using buffer_use_levels<size_t>::empty;
	using buffer_use_levels<size_t>::mark_front_removed;

	inline const_pointer
	begin() const { return first(buffer); }

	inline pointer
	begin() { return first(buffer); }

	inline const_pointer
	end() const { return last(buffer); }

	inline pointer
	end() { return last(buffer); }

	inline void
	mark_tail_used(size_t n) { mark_back_appended(n); }

	template <typename Device>
	friend inline ssize_t
	write(Device & d, source_buffer & self)
	{
		ssize_t s = write(d, self.begin(), self.size());
		if (s > 0) { self.mark_front_removed(s); }
		return s;
	}
};
