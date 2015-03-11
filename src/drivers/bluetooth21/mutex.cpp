#include <cerrno>
#include <cstring>

#include "mutex.hpp"

namespace BT
{

void
takesem(sem_t & s)
{
	int r = sem_wait(&s);
	while (r != 0)
	{
		//D_ASSERT(r == -EINTR);
		dbg("takesem %i %i %s\n", r == -EINTR, r, strerror(-r));
		r = sem_wait(&s);
	}
}

}
// end of namespace BT
