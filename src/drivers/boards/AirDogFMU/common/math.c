#include <math.h>

#ifdef __cplusplus
__EXPORT int matherr(struct __exception *e)
{
	return 1;
}
#else
__EXPORT int matherr(struct exception *e)
{
	return 1;
}
#endif

