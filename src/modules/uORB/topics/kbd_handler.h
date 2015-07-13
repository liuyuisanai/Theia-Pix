/**
 * @file leash_display.h
 *
 * Definition of the leash display topic.
 */

#ifndef KDB_HANDLER_H_
#define KDB_HANDLER_H_

#include "../uORB.h"
#include <stdint.h>

/**
 * @addtogroup topics
 * @{
 */

/**
 * kdb_handler status
 */
struct kbd_handler_s {
    int currentMode;
    int buttons;
    int event;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(kbd_handler);

#endif
