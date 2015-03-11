#include <arch/irq.h>

#include "chardev_poll.hpp"
#include "chardev_poll.hpp"

namespace BT
{

struct interrupt_guard
{
	irqstate_t state;

	interrupt_guard() : state(irqsave()) {}
	~interrupt_guard() { irqrestore(state); }
};

inline void
poll_notify_unsafe(pollfd * p, pollevent_t events)
{
	// Copied from cdev.cpp

	// FIXME is semcount check required?

	/* update the reported event set */
	p->revents |= p->events & events;

	/* if the state is now interesting, wake the waiter if it's still asleep */
	/* XXX semcount check here is a vile hack; counting semphores should not be abused as cvars */
	if ((p->revents != 0) && (p->sem->semcount <= 0))
		sem_post(p->sem);
}

// void
// poll_notify(pollfd * p, pollevent_t events)
// {
// 	if (p) { poll_notify_unsafe(p, events); }
// }

inline void
poll_notify_unsafe(const PollRefList & s, pollevent_t events)
{
	poll_notify_unsafe(s.p, events);
}

// void
// poll_notify(const PollRefList & s, pollevent_t events)
// {
// 	if (not empty(s))
// 	{
// 		//
// 		// CDev::poll_notify() disables interrupts.
// 		//
// 		// Here guarding from interrupts is not required
// 		// as sem_post() protects itself
// 		// and we have at most one waiter to notify.
// 		//
// 		// interrupt_guard guard;
// 		//
//
// 		poll_notify_unsafe(s, events);
// 	}
// }

void
poll_notify_by_masks(
	PollMultiPlexer & pmp,
	poll_notify_mask_t readable,
	poll_notify_mask_t writeable
) {
	if (empty(readable) and empty(writeable))\
		return;

	interrupt_guard guard;

	for(channel_index_t i = 0; i < 8; ++i)
	{
		if (empty(pmp.channel[i]))
			continue;

		int events =
			(is_set(readable, i) ? POLLIN : 0)
			| (is_set(writeable, i) ? POLLOUT : 0);
		if (events)
			poll_notify_unsafe(pmp.channel[i], events);
	}
}

}
// end of namespace BT
