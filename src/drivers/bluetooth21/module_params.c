#include <systemlib/param/param.h>

#include "module_params.hpp"

/*
 * Telemetry mode:
 * 0 -- Plain radio modem,
 * 1 -- Long-range bluetooth.
 *
 */
PARAM_DEFINE_INT32(A_TELEMETRY_MODE, 0);

/*
 * Device ID visible to user as a bluetooth name as one of
 *   AirDog %i, AirLeash %i, PX4 %i.
 */
PARAM_DEFINE_INT32(A_DEVICE_ID, BT_PARAM_DEFAULT);

/*
 * Mode and factory address index to connect to.
 *
 * Default `-1` is the listen mode.
 *
 * A value in the range [0, n_factory_addresses) sets the connect mode
 * and the value is number of factory address to connect to.
 *
 * A value outside the range also sets the listen mode.
 *
 */
PARAM_DEFINE_INT32(A_BT_CONNECT_TO, BT_PARAM_DEFAULT);
