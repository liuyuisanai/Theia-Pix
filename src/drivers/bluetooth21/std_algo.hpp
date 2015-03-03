#pragma once

namespace BT
{

template <typename InputIt, typename OutputIt>
OutputIt
copy(InputIt first, InputIt last, OutputIt d_first)
{
	while (first != last)
	{
		*d_first = *first;
		++d_first;
		++first;
	}
	return d_first;
}

template< class InputIt, class Size, class OutputIt>
OutputIt
copy_n(InputIt first, Size count, OutputIt result)
{
	if (count > 0)
	{
		*result = *first;
		++result;
		for (Size i = 1; i < count; ++i)
		{
			++first;
			*result = *first;
			++result;
		}
	}
	return result;
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

template <typename T>
const T&
min(const T& a, const T& b) { return (b < a) ? b : a; }

}
// end of namespace BT
