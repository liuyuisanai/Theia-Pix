#include "kbd_defines.hpp"
#include "value_switch.hpp"

namespace kbd_handler {

using mask_t = pressed_mask_t;

static void
say(const char s[])
{ fprintf(stderr, "%s\n", s); }

template <mask_t> void
on_short_press() { say("Short unknown"); }

template<> void
on_short_press<UP>() { say("Short UP"); }

template<> void
on_short_press<DOWN>() { say("Short DOWN"); }

template<> void
on_short_press<CENTER>() { say("Short CENTER"); }

template<> void
on_short_press<LEFT>() { say("Short LEFT"); }

template<> void
on_short_press<RIGHT>() { say("Short RIGHT"); }

template<> void
on_short_press<POWER>() { say("Short POWER"); }

template<> void
on_short_press<PLAY>() { say("Short PLAY"); }

template <mask_t m>
struct ShortPress
{ inline void operator () () { on_short_press<m>(); } };


template <mask_t> void
on_long_press() { say("Long unknown"); }

template<> void
on_long_press<UP>() { say("Long UP"); }

template<> void
on_long_press<DOWN>() { say("Long DOWN"); }

template<> void
on_long_press<CENTER>() { say("Long CENTER"); }

template<> void
on_long_press<LEFT>() { say("Long LEFT"); }

template<> void
on_long_press<RIGHT>() { say("Long RIGHT"); }

template<> void
on_long_press<POWER>() { say("Long POWER"); }

template<> void
on_long_press<PLAY>() { say("Long PLAY"); }

template <mask_t m>
struct LongPress
{ inline void operator () () { on_long_press<m>(); } };


template <template <pressed_mask_t> class EventT>
inline void
handle_event(pressed_mask_t p)
{
	using switch_type = ValueSwitch<pressed_mask_t, EventT, ALL_BUTTONS>;
	switch_type s;
	s.process(p);
}

} // end of namespace kbd_handler
