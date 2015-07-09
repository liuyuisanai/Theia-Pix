#include <systemlib/param/param.h>

#include "module_params.hpp"

/*
 * Device ID visible to user as a bluetooth name as one of
 *   AirDog %i, AirLeash %i, PX4 %i.
 */
PARAM_DEFINE_INT32(A_DEVICE_ID, BT_PARAM_DEFAULT);

/*
 * Does telemetry port have CTS and RTS signals connected to MCU?
 * 0 -- no, anything else -- yes.
 */
PARAM_DEFINE_INT32(A_TELEMETRY_FLOW, CONFIG_TELEMETRY_HAS_CTSRTS);

/*
 * Telemetry mode:
 *    0 -- Radio modem.
 *    1 -- Long-range bluetooth.
 *    2 -- Long-range bluetooth on BL600 port, FMU #22.
 *    3 -- Radio modem on BL600 port, FMU #22.
 *    4 -- No telemetry
 *
 */
PARAM_DEFINE_INT32(A_TELEMETRY_MODE, 1 );

/*
 * Factory address index to connect to for one-connect mode in factory service mode.
 *
 * A value in the range [0, n_factory_addresses) sets the connect mode
 * and the value is number of factory address to connect to.
 *
 * '-1' means - no default address and device should look for device to connect to.
 */
PARAM_DEFINE_INT32(A_BT_CONNECT_TO, -1 );

/*
 *
 * Bluetooth21 service mode:
 * 0 -- Factory mode ( Factory testing mode. No pairing required. Predefined devices.)
 * 1 -- User mode. (Real user expierence. Pairing is required.)
 *
 */
PARAM_DEFINE_INT32(A_BT_SVC_MODE, 0 );
