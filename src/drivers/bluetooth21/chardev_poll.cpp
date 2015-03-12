#include <arch/irq.h>

#include "chardev_poll.hpp"
#include "debug.hpp"

namespace BT
{

struct interrupt_guard
{
	irqstate_t state;

	interrupt_guard() : state(irqsave()) {}
	~interrupt_guard() { irqrestore(state); }
};

inline void
poll_notify_unsafe(pollfd * p, pollevent_t re)
{
	// Copied from cdev.cpp

	// FIXME is semcount check required?

	/* update the reported event set */
	p->revents = p->events & re;

	/* if the state is now interesting, wake the waiter if it's still asleep */
	/* XXX semcount check here is a vile hack; counting semphores should not be abused as cvars */
	if ((p->revents != 0) && (p->sem->semcount <= 0))
		sem_post(p->sem);
}

inline pollevent_t
poll_revents(bool readable, bool writeable)
{ return (readable ? POLLIN : 0) | (writeable ? POLLOUT : 0); }

void
poll_notify_channel_unsafe(
	PollMultiPlexer & pmp,
	channel_index_t ch,
	bool readable,
	bool writeable
) { poll_notify_unsafe(pmp.channel[ch].p, poll_revents(readable, writeable)); }

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

		poll_notify_channel_unsafe(pmp, i,
			is_set(readable, i), is_set(writeable, i));
	}
}

}
// end of namespace BT
