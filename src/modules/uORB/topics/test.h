#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

struct debug_data_s {
    double val[8];
    bool val_used[8];
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(debug_data);
