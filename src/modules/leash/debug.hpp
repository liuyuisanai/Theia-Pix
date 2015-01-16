#pragma once

#include <cstdio>
#include <cstring>

#define say(msg)	(fprintf(stderr, "%s:%i: %s\n", strrchr(__FILE__, '/') + 1, __LINE__, msg), fflush(stderr))
#define say_f(fmt, ...)	(fprintf(stderr, "%s:%i: " fmt "\n", strrchr(__FILE__, '/') + 1, __LINE__, __VA_ARGS__), fflush(stderr))
