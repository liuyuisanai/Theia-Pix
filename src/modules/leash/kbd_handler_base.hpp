#include <type_traits>

#include "kbd_defines.hpp"
#include "value_switch.hpp"

namespace kbd_handler
{

constexpr ModeId
previous(ModeId x)
{ return x == ModeId::NONE ? x : ModeId(int(x) - 1); }


} // end of namespace kbd_handler

namespace kbd_handler { namespace details {

/*
 * Wrappers calling handle<Mode, Event, Button>::exec that type-check
 * for default case and
 * for LongPress/RepeatPress mutually exclusive definitions.
 */

template <EventKind EVENT, ModeId MODE, ButtonId BUTTON>
struct call_handle_info
{
	using handle_type = handle<MODE, EVENT, BUTTON>;

	static constexpr bool
	is_default = std::is_base_of<Default, handle_type>::value;

	static inline void
	call(App & app) { handle_type::exec(app); }
};

template <ModeId MODE, ButtonId BUTTON>
class assert_exclusive_long_and_repeated_press_type
{
	static constexpr bool
	long_is_default = call_handle_info<EventKind::LONG_PRESS, MODE, BUTTON>::is_default;
	static constexpr bool
	repeat_is_default = call_handle_info<EventKind::REPEAT_PRESS, MODE, BUTTON>::is_default;

	static_assert(long_is_default or repeat_is_default,
		/* Note: default is opposite to defined. */

		"LongPress and RepeatPress can _not_ both be defined for"
		" the same button and mode."
		/*
		 * Note: If you got the error, check
		 *   void handle(ModeX, LongPress/RepeatPress, Up/Down/...)
		 */
	);

};

template <EventKind EVENT, ModeId MODE, ButtonId BUTTON>
struct call_handle_strict : call_handle_info<EVENT, MODE, BUTTON>
{};

template <ModeId MODE, ButtonId BUTTON>
struct call_handle_strict<EventKind::LONG_PRESS, MODE, BUTTON>
	: call_handle_info<EventKind::LONG_PRESS, MODE, BUTTON>,
	  assert_exclusive_long_and_repeated_press_type<MODE, BUTTON>
{};

template <ModeId MODE, ButtonId BUTTON>
struct call_handle_strict<EventKind::REPEAT_PRESS, MODE, BUTTON>
	: call_handle_info<EventKind::REPEAT_PRESS, MODE, BUTTON>,
	  assert_exclusive_long_and_repeated_press_type<MODE, BUTTON>
{};


/*
 * std::function<void()> replacement.
 *
 * It is required as gnu stl and nuttx have conflict at ctype.h and locale.
 *
 * The replacement is to returning static function addresses.
 * The static addressed are returned by explicit typecast operator to
 * one of the following types.
 */

template <typename T>
using void_fun_1_pointer_t = void(*)(T);

template <typename T1, typename T2>
using void_fun_2_pointer_t = void(*)(T1, T2);

template <typename T1, typename T2, typename T3>
using void_fun_3_pointer_t = void(*)(T1, T2, T3);


/*
 * Helpers that choose proper handle(Event, Mode, Button)
 * by given event type and mode and button constants.
 *
 */

template <EventKind EVENT, ModeId MODE, ButtonId BUTTON>
struct resolve_handle_0
{
	using info = call_handle_strict<EVENT, MODE, BUTTON>;

	static void
	resolve (App & app) { info::call(app); }

	inline explicit
	operator void_fun_1_pointer_t<App &> () const
	{ return &resolve; }
};

template <EventKind EVENT, ModeId MODE>
struct resolve_handle_1
{
	template <ButtonId BUTTON>
	using switch_case_type = resolve_handle_0<EVENT, MODE, BUTTON>;

	using switch_result_type = void_fun_1_pointer_t<App &>;

	static void
	resolve(App & app, ButtonId b)
	{
		using value_switch_type = ValueListSwitch<
			ButtonId,
			switch_result_type,
			switch_case_type,
			-1,
			ALL_BUTTONS
		>;
		const auto call = value_switch_type::choose_by(b);
		call(app);
	}

	inline explicit
	operator void_fun_2_pointer_t<App &, ButtonId> () const
	{ return &resolve; }
};

template <EventKind EVENT>
struct resolve_handle_2
{
	template <ModeId MODE>
	using switch_case_type = resolve_handle_1< EVENT, MODE >;

	using switch_result_type = void_fun_2_pointer_t<App &, ButtonId>;

	static void
	resolve (App & app, ModeId m, ButtonId b)
	{
		using value_switch_type = ValueRangeSwitch<
			ModeId,
			switch_result_type,
			switch_case_type,
			ModeId::NONE,
			ModeId::LOWER_BOUND, ModeId::UPPER_BOUND
		>;
		const auto call = value_switch_type::choose_by(m);
		call(app, b);
	}

	inline explicit
	operator void_fun_3_pointer_t<App &, ModeId, ButtonId> () const
	{ return resolve; }
};

}} // end of namespace kbd_handler::details

namespace kbd_handler
{

/*
 * Shortcuts to resolve_handle_X types for more clear event processing code.
 */

template <EventKind EVENT>
void
handle_event(App & app, ModeId m, ButtonId b)
{
	using namespace details;
	using resolve_type = resolve_handle_2<EVENT>;
	resolve_type::resolve(app, m, b);
}

} // end of namespace kbd_handler
