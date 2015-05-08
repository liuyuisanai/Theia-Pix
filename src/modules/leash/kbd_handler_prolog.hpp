#pragma once

#include <type_traits>

#include "debug.hpp"
#include "kbd_defines.hpp"

namespace kbd_handler {

template< bool matches >
using When = typename std::enable_if< matches >::type;

template <ModeId, EventKind, ButtonId, typename When = void>
struct handle;

} // end of namespace kbd_handler
