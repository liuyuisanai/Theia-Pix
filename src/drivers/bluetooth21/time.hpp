#pragma once

#include <drivers/drv_hrt.h>

namespace BT
{
namespace Time
{

/*
 * Timestamps require a monotonic clock, that is never adjusted.
 * GPS driver adjusts system clock, so we need hrt_time.
 */

using stamp_t = hrt_abstime;
using duration_t = hrt_abstime;

inline stamp_t
now() { return hrt_absolute_time(); }

constexpr duration_t
duration_sec(uint32_t seconds) { return seconds * 1000000; }

}
// end of namespace Time
}
// end of namespace BT
