#pragma once

template <typename T, typename size_type, size_type N, typename level_type=size_type>
struct AlwaysFullBuffer
{
	level_type level;
	T data[N];

	AlwaysFullBuffer() : level{0} {}

	inline void
	fill(const T & x)
	{
		for (size_type i=0; i < N; ++i) { data[i] = x; }
	}

	friend inline void
	reset(AlwaysFullBuffer & s)
	{
		s.level = 0;
		s.fill(0);
	}

	friend inline void
	put(AlwaysFullBuffer & s, const T & x)
	{
		size_type p = s.level % N;
		s.data[p] = x;
		s.level = p + 1;
	}

	template <typename I, typename O>
	static inline O
	_copy_reverse(I ifirst, I ilast, O ofirst)
	{
		while (ifirst != ilast)
		{
			--ilast;
			*ofirst = *ilast;
			++ofirst;
		}
		return ofirst;
	}


	template <typename O>
	friend inline O
	copy_n_reverse_from(const AlwaysFullBuffer & b, O output, size_type size)
	{
		size_type first, last;
		if (size < 0) { return output; }

		last = b.level;
		first = size < last ? last - size : 0;
		output = _copy_reverse(b.data + first, b.data + last, output);

		size -= last - first;
		if (size == 0) { return output; }

		last = N;
		first = b.level;
		if (size < last - first) { first = last - size; }
		output = _copy_reverse(b.data + first, b.data + last, output);
		return output;
	}
};
