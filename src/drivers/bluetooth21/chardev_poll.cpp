//#include <arch/irq.h>

#include "chardev_poll.hpp"

namespace BT
{

//struct interrupt_guard
//{
//	irqstate_t state;
//
//	interrupt_guard() : state(irqsave()) {}
//	~interrupt_guard() { irqrestore(state); }
//};

void
poll_notify(const PollRefSet & s, pollevent_t events)
{
	if (not empty(s))
	{
		//
		// CDev::poll_notify() disables interrupts.
		//
		// Here guarding from interrupts is not required
		// as sem_post() protects itself
		// and we have at most one waiter to notify.
		//
		// interrupt_guard guard;
		//

		poll_notify(s.p, events);
	}
}

void
poll_notify(pollfd * p, pollevent_t events)
{
	if (p)
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
}

}
// end of namespace BT
