#include <nuttx/config.h>

#include <systemlib/param/param.h>

/**
 * Size of the trajectory buffer. Each point takes up 24 bytes
 *
 * @unit quantity
 * @min 100
 * @max 1000
 * @group PathFollow
 */
PARAM_DEFINE_INT32(PAFOL_BUFF_SIZE, 500);


/**
 * Optimal trajectory distance from target to drone
 *
 * @unit meters
 * @min 5
 * @max 100
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_OPT_D, 10.0f);


/**
 * Minimum altitude offset from target trajectory
 *
 * @unit meters
 * @min 0
 * @max 100
 * @group PathFollow
 */

PARAM_DEFINE_FLOAT(PAFOL_ALT_OFF, 3.0f);

/**
 * Path follow acceptance radius
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_ACC_RAD, 5.0f);

/**
 * Path follow acceptance distance to line 
 *
 * @unit meters
 * @min 0.1
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_GT_AC_DST, 2.0f);

/**
 * Path follow gate width
 *
 * @unit meters
 * @min 0.1
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_GT_WIDTH, 8.0f);


/**
 * Coefficient for integral part of velocity PID controller 
 * @min 0
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_I, 0.0f);


/**
 * Coefficient for proportional part of velocity PID controller 
 * @min 0
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_P, 0.0f);

/**
 * Coefficient for dirivative part of velocity PID controller 
 * @min 0
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_D, 0.0f);


/**
 * Follow path vel PID control integral part aditional decrease rate
 * when aditional decrease necessary. 
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_I_DR, 5.0f);

/**
 * Follow path vel PID control integral part aditional increase rate
 * when aditional increase necessary. 
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_I_IR, 5.0f);

/**
 * Follow path vel PID control integral part upper limit.
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_I_UL, 100.0f);

/**
 * Follow path vel PID control integral part lower limit.
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VPID_I_LL, -50.0f);

/**
 * Follow path going backwards distance limit
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_BW_DST_LIM, 5.0f);


