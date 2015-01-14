#define BOOST_TEST_MODULE airleash_kbd_state_test

#include <algorithm>
#include <cstdint>
#include <initializer_list>

#include <boost/test/unit_test.hpp>

#include "kbd_reader.hpp"

using std::begin;
using std::end;

constexpr size_t N = 4;
using mask_t = unsigned;
using timestamp_t = int;
using State = ButtonState<mask_t, N, timestamp_t, 1>;

void
upd(State &s, timestamp_t now, std::initializer_list<mask_t> recent_last)
{
	mask_t first_recent[N];
	if (recent_last.size() != N) { throw "Invalid test."; }
	std::reverse_copy(begin(recent_last), end(recent_last),
			begin(first_recent));
	s.update(now, first_recent);
}

BOOST_AUTO_TEST_CASE( test_one_button_no_delay )
{
	State s;
	timestamp_t t = N - 1;
	s.init(t);

	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, 0, 1, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_EQUAL( s.time_released, t );

	++t;
	upd(s, t, {0, 1, 0, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {1, 0, 1, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, 1, 1, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 2 );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {1, 1, 1, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 3 );
	BOOST_CHECK_EQUAL( s.time_released, t );

	++t;
	upd(s, t, {1, 1, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {1, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );
}

BOOST_AUTO_TEST_CASE( test_one_button_with_delays )
{
	State s;
	timestamp_t t = N - 1;
	s.init(t);

	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	t += 2;
	upd(s, t, {0, 0, 1, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_GE( s.time_released, s.time_pressed );

	t += 2;
	upd(s, t, {1, 0, 0, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	t += 3;
	upd(s, t, {1, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 3 );
	BOOST_CHECK_GE( s.time_released, s.time_pressed );

	t += N;
	upd(s, t, {1, 1, 1, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_GE( s.time_released, 0);

	t += N;
	upd(s, t, {0, 0, 0, 1});
	BOOST_CHECK_EQUAL( s.actual_button, 1 );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_GE( s.time_released, 0);
}

BOOST_AUTO_TEST_CASE( test_two_buttons_with_unrealistic_scenario )
{
	constexpr mask_t _a = 1;
	constexpr mask_t _b = 2;
	constexpr mask_t _c = 4;

	State s;
	timestamp_t t = N - 1;
	s.init(t);

	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, 0});
	BOOST_CHECK_EQUAL( s.actual_button, 0 );

	++t;
	upd(s, t, {0, 0, 0, _a});
	BOOST_CHECK_EQUAL( s.actual_button, _a );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, 0, _a, 0});
	BOOST_CHECK_EQUAL( s.actual_button, _a );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_EQUAL( s.time_released, t );

	++t;
	upd(s, t, {0, _a, 0, _b});
	BOOST_CHECK_EQUAL( s.actual_button, _b );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );


	// Unrealistic is that person could not make short presses at 50Hz.
	// Then the last short press is registered with release.

	// unreal 1
	++t;
	upd(s, t, {_a, 0, _b, _c});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, _b, _c, _c});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_b, _c, _c, 0});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 2 );
	BOOST_CHECK_EQUAL( s.time_released, t );

	// unreal 2
	t += N;
	upd(s, t, {0, 0, 0, _c});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, 0, _c, _b});
	BOOST_CHECK_EQUAL( s.actual_button, _b );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {0, _c, _b, _a});
	BOOST_CHECK_EQUAL( s.actual_button, _a );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_c, _b, _a, _a});
	BOOST_CHECK_EQUAL( s.actual_button, _a );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1 );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_b, _a, _a, _b});
	BOOST_CHECK_EQUAL( s.actual_button, _b );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_a, _a, _b, _a});
	BOOST_CHECK_EQUAL( s.actual_button, _a );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_a, _b, _a, _c});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t );
	BOOST_CHECK_EQUAL( s.time_released, 0 );

	++t;
	upd(s, t, {_b, _a, _c, 0});
	BOOST_CHECK_EQUAL( s.actual_button, _c );
	BOOST_CHECK_EQUAL( s.time_pressed, t - 1);
	BOOST_CHECK_EQUAL( s.time_released, t );
}
