#pragma once
#include <stdint.h>
#include <sys/types.h>
#include "../uORB.h"


#define DEBUG_DATA_VAL_NUM 8;

/**
 * @addtogroup topics
 * @{
 */

struct debug_data_s {
    float val[8];
};
/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(debug_data);
