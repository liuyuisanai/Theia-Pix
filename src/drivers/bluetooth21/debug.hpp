#pragma once

#ifdef CONFIG_DEBUG_BLUETOOTH21
#include <syslog.h>
#endif

namespace BT
{

template <typename ... types>
inline void
dbg(types ... args)
{
#ifdef CONFIG_DEBUG_BLUETOOTH21
	syslog(args...);
#endif
}

}
// end of namespace BT
