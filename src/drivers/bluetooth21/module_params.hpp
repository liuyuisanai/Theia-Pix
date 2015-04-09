#pragma once

#include <systemlib/param/param.h>

#define BT_PARAM_DEFAULT 0x7FffFFff
#define BT_SREG_AS_IS BT_PARAM_DEFAULT

#ifdef __cplusplus
#include <cstdint>

namespace BT
{
namespace Params
{

inline uint32_t
get(const char name[])
{
	uint32_t value;
	int r = param_get(param_find(name), &value);
	if (r == 0) { return value; }
	return BT_PARAM_DEFAULT;
}

}
// end of namespace Params
}
// end of namespace BT
#endif // __cplusplus
