#pragma once

#include <poll.h>

namespace BT
{

/*
 * chardev.hpp does not allow more than one active file handle.
 * That is why one pollfd address is enough.
 */

struct PollRefSet
{
	pollfd * p;

	PollRefSet() : p(nullptr) {}
};

inline bool
empty(const PollRefSet & s) { return s.p == nullptr; }

inline bool
full(const PollRefSet & s) { return not empty(s); }

inline void
add(PollRefSet & s, pollfd * x) { s.p = x; }

inline void
clear(PollRefSet & s) { s.p = nullptr; }

void
poll_notify(const PollRefSet & s, pollevent_t events);

void
poll_notify(pollfd * p, pollevent_t events);

}
// end of namespace BT
