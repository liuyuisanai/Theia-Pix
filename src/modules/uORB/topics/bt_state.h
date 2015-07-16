/**
 * @file bt_state.h
 * Definition of the bt_state uORB topic.
 *
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#ifndef BT_STATE_H_
#define BT_STATE_H_

#include "../uORB.h"

/**
 * @addtogroup topics @{
 */

enum GLOBAL_BT_STATE {

    INITIALIZING = 0,
    PAIRING,
    NO_PAIRED_DEVICES,
    CONNECTING,
    CONNECTED

};

struct bt_state_s {
    GLOBAL_BT_STATE global_state;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(bt_state);

#endif
