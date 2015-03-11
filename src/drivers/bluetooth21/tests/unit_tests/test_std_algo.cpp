#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <iterator>
#include <numeric>

#include <std_algo.hpp>
#include <std_iter.hpp>

BOOST_AUTO_TEST_SUITE( std_algo )

using BT::cbegin;
using BT::cend;
using std::begin;
using std::end;

BOOST_AUTO_TEST_CASE( test_copy_ints_to_ints )
{
	int src[5];
	int dst[sizeof src];

	std::iota(std::begin(src), std::end(src), 0);
	BOOST_CHECK_EQUAL( BT::copy(cbegin(src), cbegin(src), dst) - dst, 0 );
	BOOST_CHECK_EQUAL( BT::copy(cbegin(src), cend(src), dst) - dst, 5 );
	BOOST_CHECK( std::equal(cbegin(src), cend(src), dst) );
}

BOOST_AUTO_TEST_CASE( test_copy_chars_to_ints )
{
	const char src[5] = "abcd";
	int dst[sizeof src];

	BOOST_CHECK_EQUAL( BT::copy(cbegin(src), cbegin(src), dst) - dst, 0 );
	BOOST_CHECK_EQUAL( BT::copy(cbegin(src), cend(src), dst) - dst, sizeof(src) / sizeof(*src));
	BOOST_CHECK( std::equal(cbegin(src), cend(src), dst) );
}

BOOST_AUTO_TEST_SUITE_END()
