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

/* not possible to include this from modes.hpp directly due to linker errors
 * copy-pasting this enum here */
enum class ModeId : uint8_t
{
	  NONE
	,                LOWER_BOUND // ValueRangeSwitch LOWER_BOUND
	, INIT         = LOWER_BOUND
	, PREFLIGHT
	, MENU
	, CONFIRM_ARM
	, FLIGHT
	, FLIGHT_ALT
	, FLIGHT_CAM
	, SHORTCUT
	, FLIGHT_NO_SIGNAL
	,                UPPER_BOUND // ValueRangeSwitch UPPER_BOUND
};

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
