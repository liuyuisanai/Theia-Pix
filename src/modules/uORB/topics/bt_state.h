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

enum GLOBAL_STATE {

    PAIRING = 0,
    CONNECTING = 1,
    CONNECTED = 2

};

struct bt_state_s {
    GLOBAL_STATE global_state;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(bt_state);

#endif
