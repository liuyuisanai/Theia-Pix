#pragma once

#include "std_algo.hpp"

namespace BT
{

template <typename T, unsigned SIZE, typename SizeType = unsigned, typename IndexType = int>
struct PODArray
{
	using value_type = T;
	using pointer_type = T *;
	using const_pointer_type = const T *;
	using iterator = pointer_type;
	using const_iterator = const_pointer_type;
	using size_type = SizeType;

	value_type data[SIZE];

	PODArray() { fill_n(data, SIZE, value_type{}); }
	PODArray(const T (&x)[SIZE]) { copy_n(x, SIZE, data); }

	template <typename ... types>
	constexpr
	PODArray(const types & ... x) : data{T(x)...} {}

	PODArray(const PODArray & other) = default;
	PODArray & operator = (const PODArray & other) = default;

	PODArray &
	operator = (const T (&other)[SIZE])
	{
		copy_n(other, SIZE, data);
		return *this;
	}

	//PODArray(const PODArray & other)
	//{ copy_n(data, SIZE, other.data); }

	//PODArray &
	//operator = (const PODArray & other)
	//{ copy_n(data, SIZE, other.data); }

	value_type & operator [] (IndexType i) { return data[i]; }
	const value_type & operator [] (IndexType i) const { return data[i]; }

	iterator begin() { return data; }
	const_iterator begin() const { return data; }

	iterator end() { return data + SIZE; }
	const_iterator end() const { return data + SIZE; }

	bool
	operator == (const PODArray & other) const
	{ return equal_n(data, SIZE, other.data); }

	bool
	operator != (const PODArray & other) const
	{ return not (*this == other); }

	friend constexpr size_type
	capacity(const PODArray &) { return SIZE; }
	friend constexpr size_type
	size(const PODArray &) { return SIZE; }
};

}
// end of namespace BT
