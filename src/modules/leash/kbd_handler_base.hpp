#include <type_traits>

#include "kbd_defines.hpp"
#include "value_switch.hpp"

namespace kbd_handler
{

constexpr ModeId
previous(ModeId x)
{ return x == ModeId::NONE ? x : ModeId(int(x) - 1); }

/*
 * handle(Event, Mode, Button) default placeholder.
 *
 * It should never be called.
 *
 * It is used to check if specific handle(Event, Mode, Button) is defined.
 * The check is done by return type.
 */
template <typename ... T>
Default
handle(const T & ...);

} // end of namespace kbd_handler

namespace kbd_handler { namespace details {

/*
 * Wrappers calling handle(Event, Mode, Button) that type-check
 * for default case and
 * for LongPress/RepeatPress mutually exclusive definitions.
 */

template <typename EventT, typename ModeT, typename ButtonT>
struct call_handle_info
{
	using event_type  = EventT;
	using mode_type   = ModeT;
	using button_type = ButtonT;

	static inline void
	do_call(std::true_type default_case)
	{ handle_default(); }

	static inline void
	do_call(std::false_type)
	{ handle(mode_type{}, event_type{}, button_type{}); }

	using handle_result_type =
		decltype(handle(mode_type{}, event_type{}, button_type{}));

	using // either std::false_type or std::true_type
	is_default_type = std::is_same<handle_result_type, Default>;

	static constexpr bool
	is_default = is_default_type::value;

	static inline void
	call() { do_call(is_default_type{}); }
};

template <typename ModeT, typename ButtonT>
class assert_exclusive_long_and_repeated_press_type
{
	static constexpr bool long_is_default =
		call_handle_info<LongPress, ModeT, ButtonT>::is_default;
	static constexpr bool repeat_is_default =
		call_handle_info<RepeatPress, ModeT, ButtonT>::is_default;

	static_assert(
		long_is_default or repeat_is_default,
		/* Note: default is opposite to defined. */

		"LongPress and RepeatPress can _not_ both be defined for"
		" the same button and mode."
		/*
		 * Note: If you got the error, check
		 *   void handle(ModeX, LongPress/RepeatPress, Up/Down/...)
		 */
	);

};

template <typename EventT, typename ModeT, typename ButtonT>
struct call_handle_strict : call_handle_info<EventT, ModeT, ButtonT>
{};

template <typename ModeT, typename ButtonT>
struct call_handle_strict<LongPress, ModeT, ButtonT>
	: call_handle_info<LongPress, ModeT, ButtonT>,
	  assert_exclusive_long_and_repeated_press_type<ModeT, ButtonT>
{};

template <typename ModeT, typename ButtonT>
struct call_handle_strict<RepeatPress, ModeT, ButtonT>
	: call_handle_info<RepeatPress, ModeT, ButtonT>,
	  assert_exclusive_long_and_repeated_press_type<ModeT, ButtonT>
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

using void_fun_0_pointer_type = void(*)();

template <typename T>
using void_fun_1_pointer_type = void(*)(T);

template <typename T1, typename T2>
using void_fun_2_pointer_type = void(*)(T1, T2);


/*
 * Helpers that choose proper handle(Event, Mode, Button)
 * by given event type and mode and button constants.
 *
 */

template <typename EventT, typename ModeT, ButtonId b>
struct resolve_handle_0
{
	using event_type  = EventT;
	using mode_type   = ModeT;
	using button_type = typename ButtonTypeMap< b >::type;

	using info = call_handle_strict<event_type, mode_type, button_type>;

	static void
	resolve () { info::call(); }

	inline explicit
	operator void_fun_0_pointer_type () const
	{ return &resolve; }
};

template <typename EventT, typename ModeT>
struct resolve_handle_1
{
	using event_type = EventT;
	using mode_type  = ModeT;

	template <ButtonId _b>
	using switch_case_type = resolve_handle_0<event_type, mode_type, _b>;

	//using switch_result_type = std::function<void()>;
	using switch_result_type = void_fun_0_pointer_type;

	static void
	resolve(ButtonId x)
	{
		using value_switch_type = ValueListSwitch<
			ButtonId,
			switch_result_type,
			switch_case_type,
			-1,
			ALL_BUTTONS
		>;
		const auto call = value_switch_type::choose_by(x);
		call();
	}

	inline explicit
	operator void_fun_1_pointer_type<ButtonId> () const
	{ return &resolve; }
};


template <typename EventT>
struct resolve_handle_2
{
	using event_type = EventT;

	template <ModeId m>
	using switch_case_type = resolve_handle_1<
		event_type,
		typename ModeTypeMap<m>::type
	>;

	//using switch_result_type = std::function<void()>;
	using switch_result_type = void_fun_1_pointer_type<ButtonId>;

	static void
	resolve (ModeId m, ButtonId b)
	{
		using value_switch_type = ValueRangeSwitch<
			ModeId,
			switch_result_type,
			switch_case_type,
			ModeId::NONE,
			ModeId::LOWER_BOUND, ModeId::UPPER_BOUND
		>;
		const auto call = value_switch_type::choose_by(m);
		call();
	}

	inline explicit
	operator void_fun_2_pointer_type<ModeId, ButtonId> () const
	{ return resolve; }
};

}} // end of namespace kbd_handler::details

namespace kbd_handler
{

/*
 * Shortcuts to resolve_handle_X types for more clear event processing code.
 */

template <typename EventT>
void
handle_event(ButtonId b)
{
	using namespace details;
	using resolve_type = resolve_handle_1<EventT, ModeA>;
	resolve_type::resolve(b);
}

template <typename EventT>
void
handle_event(ModeId m, ButtonId b)
{
	using namespace details;
	using resolve_type = resolve_handle_2<EventT>;
	resolve_type::resolve(m, b);
}

} // end of namespace kbd_handler
