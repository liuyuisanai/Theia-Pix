
/**
 * @file calibrator.h
 *
 * Definition of the leash display topic.
 */

#pragma once

#include "../uORB.h"
#include "../../airdog/calibrator/calibration_commons.hpp"
#include <stdint.h>

/**
 * @addtogroup topics
 * @{
 */

enum CalibratorStatus
{
    CALIBRATOR_DETECTING_SIDE,
    CALIBRATOR_CALIBRATING,
    CALIBRATOR_FINISH,
};

/**
 * calibrator status
 */
struct calibrator_s {
    int status;
    int remainingAxesCount;
    calibration::CALIBRATION_RESULT result;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(calibrator);
