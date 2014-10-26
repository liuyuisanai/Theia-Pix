#include <nuttx/config.h>
#include <math.h>

__EXPORT int
#ifdef __cplusplus
matherr(struct __exception *e)
#else
matherr(struct exception *e)
#endif
{
	return 1;
}
