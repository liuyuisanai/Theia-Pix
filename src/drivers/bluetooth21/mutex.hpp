#include <semaphore.h>

#include <cerrno>

#include "debug.hpp"

namespace BT
{

void takesem(sem_t &);

/* Mutex */

struct MutexSem
{
	sem_t nuttx_sem;

	MutexSem() { sem_init(&nuttx_sem, 0, 1); }
	~MutexSem() { sem_destroy(&nuttx_sem); }

	inline void
	lock() { takesem(nuttx_sem); }

	inline bool
	try_lock_interruptable()
	{
		auto r = sem_wait(&nuttx_sem);
		if (r != 0) { ASSERT(r == -EINTR); }
		return r == 0;
	};

	inline void
	unlock() { sem_post(&nuttx_sem); }
};

/* std-like Guards */

struct lock_guard
{
	MutexSem & mutex;

	explicit
	lock_guard(MutexSem & m) : mutex(m) { mutex.lock(); }

	~lock_guard() { mutex.unlock(); }

	lock_guard(const lock_guard &) = delete;
	lock_guard & operator = (const lock_guard &) = delete;
};


struct lock_guard_interruptable
{
	MutexSem & mutex;
	const bool locked;

	explicit
	lock_guard_interruptable(MutexSem & m)
	: mutex(m)
	, locked(mutex.try_lock_interruptable())
	{}

	~lock_guard_interruptable() { if (locked) mutex.unlock(); }

	lock_guard_interruptable(const lock_guard_interruptable &) = delete;
	lock_guard_interruptable & operator = (const lock_guard_interruptable &) = delete;
};

}
// end of namespace BT
