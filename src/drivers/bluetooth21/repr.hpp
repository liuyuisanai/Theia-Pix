#pragma once

#include <sys/types.h>

#include "debug.hpp"

namespace BT
{

inline char
hex_digit(char x)
{
	x &= 0x0f;
	return x + (x < 10 ? '0' : 'a' - 10);
}

template <typename Device>
void
write_repr_char(Device &dev, char ch, bool put_space_before)
{
	//if (put_space_before) { write(dev, " ", 1); }
	//if (ch <= ' ' or ch == '\\') {
	//	char buf[] = {'\\', 'x', hex_digit(ch >> 4), hex_digit(ch)};
	//	write(dev, buf, sizeof(buf));
	//}
	//else { write(dev, (const void *)&ch, 1); }

	//char buf[] = {' ', '\\', 'x', hex_digit(ch >> 4), hex_digit(ch)};
	{
		char buf[3] = {' ', hex_digit(ch >> 4), hex_digit(ch)};
		ssize_t r = ::write(dev, buf, sizeof(buf));
		if (r < 0) { dbg_perror("write_repr_char a"); }
	}
	if (' ' <= ch and ch < '\x7f') {
		char buf[3] = {'(', ch, ')'};
		ssize_t r = ::write(dev, buf, sizeof(buf));
		if (r < 0) { dbg_perror("write_repr_char b"); }
	}
}

template <typename Device>
void
write_repr(Device &dev, const void *buf, std::size_t buf_size)
{
	auto p = static_cast<const char *>(buf);
	if (buf_size > 0) {
		write_repr_char(dev, *p, false);
		for(std::size_t i = 1; i < buf_size; ++i)
			write_repr_char(dev, p[i], true);
		char nl = '\n';
		ssize_t r = write(dev, &nl, 1);
		if (r < 0) { dbg_perror("write_repr"); }
	}
}

template <typename InputIt, typename InputSize, typename Space, typename OuputIt>
OuputIt
repr_n(InputIt first, InputSize n, const Space space, OuputIt out)
{
	static_assert(sizeof(*first) == 1, "Only one byte types are suported.");

	if (n > 0)
	{
		while (n > 1)
		{
			uint8_t x = *first;
			*out = hex_digit(x >> 4);
			++out;
			*out = hex_digit(x);
			++out;
			*out = space;
			++out;

			++first;
			--n;
		}

		uint8_t x = *first;
		*out = hex_digit(x >> 4);
		++out;
		*out = hex_digit(x);
		++out;
	}

	return out;
}

}
// end of namespace BT
