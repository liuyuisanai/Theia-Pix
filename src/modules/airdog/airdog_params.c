/**
 * @file airdog_params.c
 * Airdog specific params.
 *
 * @author Kirils Sivokozs
 */

#include <systemlib/param/param.h>

/**
 * Minimum altitude for manual follow
 *
 * Minimum altitude in manual follow. Recommended to be in range from 3 to 10.
 *
 * @min 0.0
 * @max MAX_FLOAT
 * @group Airdog
 */
PARAM_DEFINE_FLOAT(AIRD_MIN_MF_ALT, 3.0f);

/**
 * Airdog step to adjust position
 *
 * in LOITER mode
 *
 * @unit meters
 * @group AirDog
 */
PARAM_DEFINE_FLOAT(AIRD_LOITER_STEP, 2.0f);

/**
 * Airdog step to adjust position
 *
 * in LOITER mode
 *
 * @unit meters
 * @group AirDog
 */
PARAM_DEFINE_FLOAT(AIRD_BAT_WARN, 40.0f);

/**
 * Airdog step to adjust position
 *
 * in LOITER mode
 *
 * @unit meters
 * @group AirDog
 */
PARAM_DEFINE_FLOAT(AIRD_BAT_FS, 10.0f);

/*Custom binded device id.*/
PARAM_DEFINE_INT32(AIRD_BINDED_ID, 4);

/*Custom binded device id.*/
PARAM_DEFINE_INT32(AIRD_TRAINER_ID, 10);

/*Custom binded device id.*/
PARAM_DEFINE_INT32(AIRD_PITCH_DOWN, 0);

/*Enable if should check for external magnetometer*/
PARAM_DEFINE_INT32(AIRD_CHECK_MAG, 0);

/*Enable automatic magnetic declination setting from coordinates*/
PARAM_DEFINE_INT32(AIRD_AUTO_MAG, 1);
