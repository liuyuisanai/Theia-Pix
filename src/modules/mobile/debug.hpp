#pragma once

#include <dog_debug.hpp>

#define dbg(...) DOG_PRINT(__VA_ARGS__)
#define dbg_perror(fmt, ...) \
		dbg(fmt ": %i: %s\n", ##__VA_ARGS__, errno, strerror(errno));
