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
PARAM_DEFINE_FLOAT(PAFOL_OK_DIST, 10.0f);

/**
 * Difference between absolute minimum distance and desired distance
 *
 * @unit meters
 * @min 0
 * @max 95
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_MIN_TO_OK, 5.0f);

/**
 * Coefficient for maximum distance. Maximum distance = "desired distance"*coefficient
 *
 * @unit coefficient
 * @min 1
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_MAX_COEF, 1.5f);

/**
 * Safety switch distance. Drone won't even try to get to a point that is closer than this to target
 *
 * @unit meters
 * @min 5
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_SAFE_DIST, 10.0f);

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
 * Step for "closer" and "farther" commands
 *
 * @unit meters
 * @min 0.2
 * @max 50
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_DIST_STEP, 1.0f);

/**
 * Step for "up" and "down" commands
 *
 * @unit meters
 * @min 0.2
 * @max 20
 * @group PathFollow
 */
PARAM_DEFINE_FLOAT(PAFOL_ALT_STEP, 1.0f);

// 0 or 1. 1 - simple follower, 0 - L1 follower. Temporary.
PARAM_DEFINE_INT32(PAFOL_MODE, 0);


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
