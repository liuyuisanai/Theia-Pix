#pragma once

#include <assert.h>  // NuttX assert.h defines ASSERT
#include <cerrno>
#include <cstring>

#ifdef CONFIG_DEBUG_BLUETOOTH21
#include <syslog.h>
#endif

#ifdef CONFIG_DEBUG_BLUETOOTH21
#define D_ASSERT(...) ASSERT( __VA_ARGS__ )
#else
#define D_ASSERT(...)
#endif

namespace BT
{

template <typename ... types>
inline void
dbg(const char fmt[], const types & ... args)
{
#ifdef CONFIG_DEBUG_BLUETOOTH21
	int save_errno = errno;

#ifdef MODULE_COMMAND
# define MODULE_COMMAND_XSTR(X) #X
# define MODULE_COMMAND_STR(X) MODULE_COMMAND_XSTR(X)
	// syslog does not append newlines itself.
	syslog(MODULE_COMMAND_STR(MODULE_COMMAND) ": ");

	// Restore errno as an arg... could be const int & to errno.
	errno = save_errno;
#endif

	syslog(fmt, args...);
	errno = save_errno;
#endif
}

inline void
dbg_perror(const char ref[])
{ dbg("%s: %i %s.\n", ref, errno, strerror(errno)); }

}
// end of namespace BT
