#pragma once

namespace BT
{

template <typename InputIt, typename OutputIt>
OutputIt
copy(InputIt first, InputIt last, OutputIt d)
{
	while (first != last)
	{
		*d = *first;
		++d;
		++first;
	}
	return d;
}

template< class InputIt, class Size, class OutputIt>
OutputIt
copy_n(InputIt first, Size count, OutputIt d)
{
	if (count > 0)
	{
		*d = *first;
		++d;
		for (Size i = 1; i < count; ++i)
		{
			++first;
			*d = *first;
			++d;
		}
	}
	return d;
}

template< class ForwardIt, class T >
void
fill(ForwardIt first, ForwardIt last, const T& value)
{
	while (first != last)
	{
		*first = value;
		++first;
	}
}

template<class OutputIt, class Size, class T>
OutputIt
fill_n(OutputIt first, Size count, const T& value)
{
	for (Size i = 0; i < count; i++)
	{
		*first = value;
		++first;
	}
	return first;
}

template<class InputIt1, class Size1, class InputIt2>
bool equal_n(InputIt1 first1, Size1 n, InputIt2 first2)
{
    while (n != 0) {
        if (not (*first1 == *first2)) { return false; }
	--n;
	++first1;
	++first2;
    }
    return true;
}

template <typename T>
const T&
min(const T& a, const T& b) { return (b < a) ? b : a; }

}
// end of namespace BT
