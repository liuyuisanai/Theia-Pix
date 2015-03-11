#pragma once

#include <sys/types.h>

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
		write(dev, buf, sizeof(buf));
	}
	if (' ' <= ch and ch < '\x7f') {
		char buf[3] = {'(', ch, ')'};
		write(dev, buf, sizeof(buf));
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
		write(dev, &nl, 1);
	}
}
