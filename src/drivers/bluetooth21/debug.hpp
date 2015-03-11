#pragma once

#include <assert.h>  // NuttX assert.h defines ASSERT

#ifdef CONFIG_DEBUG_BLUETOOTH21
#include <syslog.h>
#endif

#define D_ASSERT ASSERT
//#ifdef CONFIG_DEBUG_BLUETOOTH21
//#define D_ASSERT(...) ASSERT
//#else
//#define D_ASSERT
//#endif

namespace BT
{

template <typename ... types>
inline void
dbg(const char fmt[], types ... args)
{
#ifdef CONFIG_DEBUG_BLUETOOTH21
	syslog(fmt, args...);
#endif
}

}
// end of namespace BT
