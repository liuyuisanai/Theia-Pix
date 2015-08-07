#pragma once

#include <systemlib/param/param.h>

#include <cstdint>

namespace AirDog
{
namespace activation
{

static inline param_t
act_p() { return ::param_find("SYS_ACT"); }

static inline uint8_t
get()
{
	uint32_t value;
	param_t p = act_p();
	if (p != PARAM_INVALID)
	{
		int r = param_get(p, &value);
		if (r == 0) { return value <= 0xFF ? value : 0; }
	}
	return 0;
}

static inline bool
set_store(uint8_t level)
{
	param_t p = act_p();
	bool ok = p != PARAM_INVALID;
	if (ok)
	{
		uint32_t value = level;
		int r = param_set(p, &value);
		if (r == 0)
		{
			r = param_save_default();
			ok = r == 0;
		}
	}
	return ok;
}

}
// end of namespace activation
}
// end of namespace AirDog
