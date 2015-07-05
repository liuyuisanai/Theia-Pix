#pragma once

#include <utility>

#include <uORB/uORB.h>

template <typename T, const orb_id_t META>
struct Subscription
{
	int h;

	inline
	Subscription() : h(orb_subscribe(META)) {}

	inline
	~Subscription() { orb_unsubscribe(h); }
};

template <typename T, const orb_id_t META>
inline T &&
orb_read(Subscription<T, META> self)
{
	T x;
	orb_copy(META, self.h, &x);
	return std::move(x);
}
