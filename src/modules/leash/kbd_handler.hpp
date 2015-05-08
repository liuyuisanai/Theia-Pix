#pragma once

#include "debug.hpp"
#include "kbd_defines.hpp"
#include "kbd_handler_prolog.hpp"
#include "kbd_power_off.hpp"

#if CONFIG_BOARD_REVISION <= 4
# include "revision/004/kbd.hpp"
#else
# include "revision/005/kbd.hpp"
#endif

namespace kbd_handler {

/*
 * This is tricky `handle' definition.
 *
 * It partially duplicates definition in kbd_handler_prolog.hpp.
 * I don't know correct way to provide
 *
 * Only this one should be inherited from Default.
 */
template <ModeId, EventKind, ButtonId, typename When>
struct handle : Default
{ static void exec(App&) { say("unknown"); } }; // TODO tone bzzz

} // end of namespace kbd_handler

/* kbd_handler_base.hpp should be included after all definitions ready. */
#include "kbd_handler_base.hpp"
