/**
 * @file leash_display.h
 *
 * Definition of the leash display topic.
 */

#ifndef LEASH_DISPLAY_H_
#define LEASH_DISPLAY_H_

#include "../uORB.h"
#include <stdint.h>

/**
 * @addtogroup topics
 * @{
 */


enum {
    LEASHDISPLAY_NONE,
    LEASHDISPLAY_LOGO,
    LEASHDISPLAY_MAIN
};


/**
 * leash display status
 */
struct leash_display_s {
    int screenId;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(leash_display);

#endif
