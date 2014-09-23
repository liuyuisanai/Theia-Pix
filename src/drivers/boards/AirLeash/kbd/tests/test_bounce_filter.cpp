#define BOOST_TEST_MODULE airleash_kbd_state_test

#include <cstdint>

#include <boost/test/unit_test.hpp>

#include "bounce_filter.hpp"

using BounceFilter = airleash_kbd::BounceFilter<>;

constexpr unsigned A = 0b01;
constexpr unsigned B = 0b10;

BOOST_AUTO_TEST_CASE( press_keep_release_keep )
{
	BounceFilter f;

	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( A ), 0 ); // it could be bounce
	BOOST_CHECK_EQUAL( f( A ), A );
	BOOST_CHECK_EQUAL( f( A ), A );
	BOOST_CHECK_EQUAL( f( A ), A );
	BOOST_CHECK_EQUAL( f( A ), A );
	BOOST_CHECK_EQUAL( f( 0 ), A ); // it could be bounce
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
}

BOOST_AUTO_TEST_CASE( press_press_and_release_almost_at_once )
{
	BounceFilter f;

	BOOST_CHECK_EQUAL( f( 0|0 ), 0|0 );
	BOOST_CHECK_EQUAL( f( 0|A ), 0|0 ); // it could be bounce
	BOOST_CHECK_EQUAL( f( 0|A ), 0|A );
	BOOST_CHECK_EQUAL( f( B|A ), 0|A ); // it could be bounce
	BOOST_CHECK_EQUAL( f( B|A ), B|A );
	BOOST_CHECK_EQUAL( f( 0|A ), B|A ); // it could be bounce at B
	BOOST_CHECK_EQUAL( f( 0|0 ), 0|A ); // it could be bounce at A
	BOOST_CHECK_EQUAL( f( 0|0 ), 0|0 );
}

BOOST_AUTO_TEST_CASE( press_almost_at_once_press_and_release )
{
	BounceFilter f;

	BOOST_CHECK_EQUAL( f( 0|0 ), 0|0 );
	BOOST_CHECK_EQUAL( f( 0|A ), 0|0 ); // it could be bounce at B
	BOOST_CHECK_EQUAL( f( B|A ), 0|A ); // it could be bounce at A
	BOOST_CHECK_EQUAL( f( B|A ), B|A );
	BOOST_CHECK_EQUAL( f( 0|0 ), B|A ); // it could be bounce
	BOOST_CHECK_EQUAL( f( 0|0 ), 0|0 );
}

BOOST_AUTO_TEST_CASE( bounce_pressing )
{
	BounceFilter f;

	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( A ), 0 );
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( A ), 0 );
	BOOST_CHECK_EQUAL( f( 0 ), 0 );
	BOOST_CHECK_EQUAL( f( A ), 0 );
	BOOST_CHECK_EQUAL( f( A ), A ); // pressed
}

BOOST_AUTO_TEST_CASE( bounce_while_pressed )
{
	BounceFilter f;

	BOOST_CHECK_EQUAL( f( B|A ), 0|0 );
	BOOST_CHECK_EQUAL( f( B|A ), B|A ); // pressed
	BOOST_CHECK_EQUAL( f( B|0 ), B|A );
	BOOST_CHECK_EQUAL( f( 0|A ), B|A );
	BOOST_CHECK_EQUAL( f( B|0 ), B|A );
	BOOST_CHECK_EQUAL( f( 0|A ), B|A );
	BOOST_CHECK_EQUAL( f( B|A ), B|A );
	BOOST_CHECK_EQUAL( f( 0|0 ), B|A );
}
