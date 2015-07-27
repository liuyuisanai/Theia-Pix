#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * Enable automatically turning BGC motors on/off upon arming/disarming the drone.
 * Any non-zero value = enable, 0 = disable.
 */
PARAM_DEFINE_INT32(A_ARM_BGC_MOTORS, 1);
