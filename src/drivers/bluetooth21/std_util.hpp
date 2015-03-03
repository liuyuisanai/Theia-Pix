#pragma once

#include <utility>

namespace BT
{

template <typename T1, typename T2>
struct TiePair
{
	T1 &first;
	T2 &second;

	TiePair(T1 & x, T2 & y) : first(x), second(y) {}

	std::pair<T1, T2> const &
	operator = (std::pair<T1, T2> const & rhs)
	{
		first = rhs.first;
		second = rhs.second;
		return rhs;
	}
};

template <typename T1, typename T2>
inline TiePair<T1,T2>
tie(T1 & x, T2 & y) { return TiePair<T1, T2>(x, y); }

template <typename T>
void
swap(T & a, T & b)
{
	T x = a;
	a = b;
	b = x;
}

}
// end of namespace BT
