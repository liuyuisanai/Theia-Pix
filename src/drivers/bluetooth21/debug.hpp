#pragma once

#ifdef CONFIG_DEBUG_BLUETOOTH21
#include <assert.h>  // NuttX assert.h defines ASSERT
#define D_ASSERT(...) ASSERT( __VA_ARGS__ )
#else
#define D_ASSERT(...)
#endif

#include <cerrno>
#include <cstring>
#include <syslog.h>

#include "time.hpp"

#ifdef MODULE_COMMAND
# define MODULE_COMMAND_XSTR(X) #X
# define MODULE_COMMAND_STR(X) MODULE_COMMAND_XSTR(X)
# define DEBUG_BLUETOOTH21_PREFIX MODULE_COMMAND_STR(MODULE_COMMAND) ": "
#else
# define DEBUG_BLUETOOTH21_PREFIX
#endif

// dbg() uses only lower half of useconds as debug session
// is unlikely to be several hours long.
#define log_printf(...) do {                       \
	const int save_errno = errno;              \
	syslog(DEBUG_BLUETOOTH21_PREFIX "%10u: ",  \
		(unsigned)BT::Time::now());        \
	errno = save_errno;                        \
	syslog(__VA_ARGS__);                       \
	errno = save_errno;                        \
} while (false)

#define log_err(...) log_printf(__VA_ARGS__)
#define log_info(...) log_printf(__VA_ARGS__)

#define dbg_perror(ref) log_printf("%s: %i %s.\n", ref, errno, strerror(errno))

#ifdef CONFIG_DEBUG_BLUETOOTH21

#define dbg(...) log_printf(__VA_ARGS__)

#else

// Avoid unused-variable warnings/errors.
template <typename ... types>
inline void
dbg(const char fmt[], const types & ... args) {}

#ifndef dbg_perror
inline void
dbg_perror(const char ref[])
{}
#endif

#endif
