#include <nuttx/config.h>

#include <systemlib/param/param.h>

/**
 * cable park mode velocity feed-forward
 *
 * Target velocity feed-forward in cable park mode.
 *
 * @min 0.0
 * @max 1.0
 * @group Airdog params, cablepark
 */
PARAM_DEFINE_FLOAT(EXFW_PITCH_P, 0.2f);

/**
 * cable park mode acceleration
 *
 * @unit meters/sec
 * @group Airdog params, cablepark
 */
PARAM_DEFINE_FLOAT(A_CP_MIN_ACC, 1.0f);
