#pragma once

#include <poll.h>

#include "bt_types.hpp"

namespace BT
{

struct PollRefList
{
	/*
	 * chardev.hpp does not allow more than one active file handle.
	 * That is why one pollfd address is enough.
	 */
	pollfd * p;

	PollRefList() : p(nullptr) {}
};


inline bool
empty(const PollRefList & s) { return s.p == nullptr; }

inline bool
full(const PollRefList & s) { return not empty(s); }

inline void
add(PollRefList & s, pollfd * x) { s.p = x; }

inline void
clear(PollRefList & s) { s.p = nullptr; }


struct PollMultiPlexer
{
	PollRefList channel[8];

	PollRefList &
	operator [] (channel_index_t i) { return channel[i]; }
};

using poll_notify_mask_t = channel_mask_t;

void
poll_notify_by_masks(
	PollMultiPlexer & pmp,
	poll_notify_mask_t readable,
	poll_notify_mask_t writeable
);

}
// end of namespace BT
