#pragma once

#include <cstdlib>

namespace BT
{

inline bool
streq(const char a[], const char b[]) { return strcmp(a, b) == 0; }

inline bool
parse_uint32(const char s[], uint32_t &n, int base=10)
{
	char * tail;
	n = std::strtoul(s, &tail, base);
	return tail != s and *tail == '\0';
}

}
// end of namespace BT
