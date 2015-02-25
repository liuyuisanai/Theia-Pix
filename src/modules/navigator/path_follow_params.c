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
 * Minimum desired distance from target
 *
 * @unit meters
 * @min 5
 * @max 100
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_MIN_OK_D, 10.0f);


/**
 * Desired distance from target
 *
 * @unit meters
 * @min 5
 * @max 100
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_OK_D, 10.0f);


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
 * Velocity error coif.
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VEL_E_C, 0.5f);


/**
 * Velocity reaction time.
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VEL_R_T, 0.5f);

/**
 * Velocity error function growth power.
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VEL_E_GP, 1.0f);


/**
 * Velocity reaction time when speed should be decreased
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VEL_R_TD, 0.5f);

/**
 * Velocity error function growth power when speed should be decreased
 *
 * @unit meters
 * @min 0.1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_VEL_E_GPD, 2.0f);


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
PARAM_DEFINE_FLOAT(PAFOL_AC_DST_LN, 2.0f);

/**
 * Path follow acceptance distance to point 
 *
 * @unit meters
 * @min 0.1
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_AC_DST_PT, 8.0f);


/**
 * Path follow speed when the drone is allowed to stop imidiatelly
 *
 * @unit meters
 * @min 0.1
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_STOP_SPD, 2.0f);

