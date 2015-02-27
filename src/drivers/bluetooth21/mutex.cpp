#include <cerrno>

#include "mutex.hpp"

namespace BT
{

void
takesem(sem_t & s)
{
	auto r = sem_wait(&s);
	while (r != 0)
	{
		assert(get_errno() == EINTR);
		r = sem_wait(&s);
	}
}

}
// end of namespace BT
