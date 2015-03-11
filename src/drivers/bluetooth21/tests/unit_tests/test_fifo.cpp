#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <numeric>

#include <cstdio>

#include "fifo.hpp"

BOOST_AUTO_TEST_SUITE( fifo )

using BT::FIFO;
using BT::cbegin;
using BT::cend;
using std::begin;
using std::end;
//using std::cbegin;
//using std::cend;

BOOST_AUTO_TEST_CASE( test_basic )
{
	FIFO<32> q;
	std::array<uint8_t, 32> data;

	std::iota(begin(data), end(data), 0);
	BOOST_CHECK_EQUAL( *begin(data), 0 );
	BOOST_CHECK_EQUAL( *std::prev(end(data)), 31 );

	BOOST_CHECK( empty(q) );
	BOOST_CHECK_EQUAL( size(q), 0 );
	BOOST_CHECK_EQUAL( space_available(q), capacity(q) );

	insert_end_unsafe(q, begin(data), end(data));

	BOOST_CHECK_EQUAL( cend(q) - cbegin(q), 32 );
	BOOST_CHECK_EQUAL( std::prev(cend(q)) - cbegin(q), (cend(q) - cbegin(q)) - 1);

	BOOST_CHECK( not empty(q) );
	BOOST_CHECK_EQUAL( space_available(q), 0 );
	BOOST_CHECK_EQUAL( size(q), capacity(q) );
	BOOST_CHECK( std::equal(begin(data), end(data), cbegin(q)) );
	BOOST_CHECK_EQUAL( *cbegin(q), 0 );
	BOOST_CHECK_EQUAL( *std::prev(cend(q)), (uint8_t)31 );

	erase_begin(q, 10);

	BOOST_CHECK( not empty(q) );
	BOOST_CHECK_EQUAL( space_available(q), 0 ); /* Head is not available. */
	BOOST_CHECK_EQUAL( size(q), capacity(q) - 10 );
	BOOST_CHECK_EQUAL( *cbegin(q), 10 );
	BOOST_CHECK_EQUAL( *std::prev(cend(q)), 31 );

	pack(q);

	BOOST_CHECK( not empty(q) );
	BOOST_CHECK_EQUAL( space_available(q), 10 );
	BOOST_CHECK_EQUAL( size(q), capacity(q) - 10 );
	BOOST_CHECK_EQUAL( *cbegin(q), 10 );
	BOOST_CHECK_EQUAL( *std::prev(cend(q)), 31 );

	clear(q);

	BOOST_CHECK( empty(q) );
	BOOST_CHECK_EQUAL( size(q), 0 );
	BOOST_CHECK_EQUAL( space_available(q), capacity(q) );
}

BOOST_AUTO_TEST_CASE( test_erase_begin_more_than_exists )
{
	FIFO<32> q;

	BOOST_CHECK( empty(q) );
	BOOST_CHECK_EQUAL( size(q), 0 );

	insert_end_n(q, 10, 0);

	BOOST_CHECK( not empty(q) );
	BOOST_CHECK_EQUAL( size(q), 10 );

	erase_begin(q, 100);

	BOOST_CHECK( empty(q) );
	BOOST_CHECK_EQUAL( size(q), 0 );
	BOOST_CHECK_EQUAL( space_available(q), capacity(q) );
}

BOOST_AUTO_TEST_SUITE_END()
