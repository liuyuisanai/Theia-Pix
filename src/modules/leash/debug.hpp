#pragma once

#include <cstdio>
#include <cstring>

#define say(msg)	(fprintf(stderr, "%s:%i: %s\n", strrchr(__FILE__, '/') + 1, __LINE__, msg), fflush(stderr))
