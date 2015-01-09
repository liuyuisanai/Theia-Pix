#pragma once

inline char
hex_digit(char x)
{
	x &= 0x0f;
	return x + (x < 10 ? '0' : 'a' - 10);
}

template <typename Device>
inline ssize_t
write_repr_char(Device dev, char ch, bool put_space_before)
{
	if (put_space_before)
	{
		ssize_t s = write(dev, " ", 1);
		if (s < 0) { return s; }
	}

	if (ch <= ' ' or ch == '\\')
	{
		char buf[] = {'\\', 'x', hex_digit(ch >> 4), hex_digit(ch)};
		return write(dev, buf, sizeof(buf));
	}

	if (put_space_before)
	{
		char buf[] = {ch, '(', '\\', 'x', hex_digit(ch >> 4), hex_digit(ch), ')'};
		return write(dev, buf, sizeof(buf));
	}

	return write(dev, (const void *)&ch, 1);
}

template <typename Device>
ssize_t
write_repr(Device dev, const void *buf, size_t buf_size)
{
	const char * const data = reinterpret_cast<const char *>(buf);

	ssize_t s = write_repr_char(dev, data[0], true);
	if (s < 0) { return s; }

	size_t i = 1;
	do {
		s = write_repr_char(dev, data[i], true);
		++i;
	} while (i < buf_size and s > 0);
	return i;
}
