#pragma once

#include <assert.h>  // NuttX assert.h defines ASSERT

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
#ifdef MODULE_COMMAND
# define MODULE_COMMAND_XSTR(X) #X
# define MODULE_COMMAND_STR(X) MODULE_COMMAND_XSTR(X)
	// syslog does not append newlines itself.
	syslog(MODULE_COMMAND_STR(MODULE_COMMAND) ": ");
#endif
	syslog(fmt, args...);
#endif
}

}
// end of namespace BT
