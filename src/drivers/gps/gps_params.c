#include <nuttx/config.h>
#include <systemlib/param/param.h>

/**
 * GPS message delay interval in ms
 * If set to 0, will use default value in ubx.h
 *
 * @min 0
 * @group GPS
 */
PARAM_DEFINE_INT32(GPS_UBX_INTERVAL, 0);

/**
 * GPS dynamics model
 * 0 Portable, 2 Stationary, 3 Pedestrian, 4 Automotive, 5 Sea,
 * 6 Airborne <1g, 7 Airborne <2g, 8 Airborne <4g
 * If set to -1, will use default value in ubx.h
 *
 * @min -1
 * @max 8
 * @group GPS
 */
PARAM_DEFINE_INT32(GPS_UBX_DYNAMICS, -1);
