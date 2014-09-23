#pragma once

namespace airleash_kbd {

static constexpr unsigned
count_args()
{ return 0; }

template <typename T1, typename ... T>
static constexpr unsigned
count_args(T1, T ... args)
{ return 1 + count_args(args...); }

} // end of namespace airleash_kbd
