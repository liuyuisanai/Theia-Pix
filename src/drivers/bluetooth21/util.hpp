#pragma once

#include <cstring>

namespace BT
{

inline bool
streq(const char a[], const char b[]) { return strcmp(a, b) == 0; }

}
// end of namespace BT
