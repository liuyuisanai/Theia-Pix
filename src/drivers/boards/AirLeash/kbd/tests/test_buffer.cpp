#define BOOST_TEST_MODULE airleash_kbd_state_test

#include <cstdint>

#include <boost/test/unit_test.hpp>

#include "buffer.hpp"

BOOST_AUTO_TEST_CASE( test_buffer_4_items )
{
	AlwaysFullBuffer<int32_t, uint16_t, 4, volatile uint16_t> b;
	uint32_t c[4], *r;

	reset(b);

	BOOST_CHECK_EQUAL( b.level, 0 );

	put(b, 1);
	BOOST_CHECK_EQUAL( b.level, 1 );

	r = copy_n_reverse_from(b, c, 2);
	BOOST_CHECK_EQUAL(r - c, 2);
	BOOST_CHECK_EQUAL(c[0], 1);
	BOOST_CHECK_EQUAL(c[1], 0);

	put(b, 2);
	BOOST_CHECK_EQUAL( b.level, 2 );

	r = copy_n_reverse_from(b, c, 3);
	BOOST_CHECK_EQUAL(r - c, 3);
	BOOST_CHECK_EQUAL(c[0], 2);
	BOOST_CHECK_EQUAL(c[1], 1);
	BOOST_CHECK_EQUAL(c[2], 0);

	put(b, 3);
	BOOST_CHECK_EQUAL( b.level, 3 );

	r = copy_n_reverse_from(b, c, 4);
	BOOST_CHECK_EQUAL(r - c, 4);
	BOOST_CHECK_EQUAL(c[0], 3);
	BOOST_CHECK_EQUAL(c[1], 2);
	BOOST_CHECK_EQUAL(c[2], 1);
	BOOST_CHECK_EQUAL(c[3], 0);

	put(b, 4);
	BOOST_CHECK_EQUAL( b.level, 4 );

	r = copy_n_reverse_from(b, c, 4);
	BOOST_CHECK_EQUAL(c[0], 4);
	BOOST_CHECK_EQUAL(c[1], 3);
	BOOST_CHECK_EQUAL(c[2], 2);
	BOOST_CHECK_EQUAL(c[3], 1);

	put(b, 5);
	BOOST_CHECK_EQUAL( b.level, 1 );

	r = copy_n_reverse_from(b, c, 4);
	BOOST_CHECK_EQUAL(c[0], 5);
	BOOST_CHECK_EQUAL(c[1], 4);
	BOOST_CHECK_EQUAL(c[2], 3);
	BOOST_CHECK_EQUAL(c[3], 2);

	r = copy_n_reverse_from(b, c, 5);
	BOOST_CHECK_EQUAL(r - c, 4);
}
