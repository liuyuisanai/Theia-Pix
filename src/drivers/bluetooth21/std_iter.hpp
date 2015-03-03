#pragma once

#include <cstdlib>

namespace BT
{

/*
 * begin()
 */

template< class C >
auto
begin( C& c ) -> decltype(c.begin())
{ return c.begin(); }

template< class C >
auto
begin( const C& c ) -> decltype(c.begin())
{ return c.begin(); }

template< class T, size_t N >
constexpr T*
begin( T (&array)[N] ) { return &array[0]; }

/*
 * end()
 */

template< class C >
auto
end( C& c ) -> decltype(c.begin())
{ return c.begin(); }

template< class C >
auto
end( const C& c ) -> decltype(c.begin())
{ return c.begin(); }

template< class T, size_t N >
constexpr T*
end( T (&array)[N] ) { return &array[N]; }

/*
 * cbegin(), cend()
 */

template< class C >
constexpr auto
cbegin( const C& c ) -> decltype(begin(c))
{ return begin(c); }

template< class C >
constexpr auto
cend( const C& c ) -> decltype(end(c))
{ return end(c); }

/*
 * distance
 */

template< class InputIt >
auto
distance( InputIt first, InputIt last ) -> decltype( last - first )
{ return last - first; }

/*
 * next
 */

template<class ForwardIt>
ForwardIt
next(ForwardIt it, ssize_t n = 1)
{ return it += n; }

}
// end of namespace BT
