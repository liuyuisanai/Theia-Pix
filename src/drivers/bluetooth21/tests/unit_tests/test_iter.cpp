#include <boost/test/unit_test.hpp>

#include <array>

#include "fifo.hpp"

BOOST_AUTO_TEST_SUITE( iter )

BOOST_AUTO_TEST_CASE( test_plain_array )
{
	char chars[10];
	const int ints[5] = {0,1,2,3,4};

	BOOST_CHECK_EQUAL( BT::begin(chars), chars );
	BOOST_CHECK_EQUAL( BT::begin(ints), ints );

	BOOST_CHECK_EQUAL( BT::cbegin(chars), chars );
	BOOST_CHECK_EQUAL( BT::cbegin(ints), ints );

	BOOST_CHECK_EQUAL( BT::end(chars), chars + 10 );
	BOOST_CHECK_EQUAL( BT::end(ints), ints + 5 );

	BOOST_CHECK_EQUAL( BT::cend(chars), chars + 10 );
	BOOST_CHECK_EQUAL( BT::cend(ints), ints + 5 );

	BOOST_CHECK_EQUAL( BT::distance(BT::begin(chars), BT::end(chars)), 10 );
	BOOST_CHECK_EQUAL( BT::distance(BT::begin(ints), BT::end(ints)), 5 );

	BOOST_CHECK_EQUAL( BT::distance(BT::cbegin(chars), BT::cend(chars)), 10 );
	BOOST_CHECK_EQUAL( BT::distance(BT::cbegin(ints), BT::cend(ints)), 5 );
}

BOOST_AUTO_TEST_CASE( test_std_array )
{
	std::array<char, 10> chars;
	const std::array<int, 5> ints{0,1,2,3,4};

	BOOST_CHECK_EQUAL( BT::end(chars), BT::next(BT::begin(chars), 10) );
	BOOST_CHECK_EQUAL( BT::end(ints), BT::next(BT::begin(ints), 5) );

	BOOST_CHECK_EQUAL( BT::cend(chars), BT::next(BT::begin(chars), 10) );
	BOOST_CHECK_EQUAL( BT::cend(ints), BT::next(BT::begin(ints), 5) );

	BOOST_CHECK_EQUAL( BT::distance(BT::begin(chars), BT::end(chars)), 10 );
	BOOST_CHECK_EQUAL( BT::distance(BT::begin(ints), BT::end(ints)), 5 );

	BOOST_CHECK_EQUAL( BT::distance(BT::cbegin(chars), BT::cend(chars)), 10 );
	BOOST_CHECK_EQUAL( BT::distance(BT::cbegin(ints), BT::cend(ints)), 5 );
}

BOOST_AUTO_TEST_SUITE_END()
