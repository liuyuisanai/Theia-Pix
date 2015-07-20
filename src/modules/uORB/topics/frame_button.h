
/**
 * @file frame_button.h
 * Definition of the frame button action topic
 *
 * @author Max Shvetsov <max@airdog.com>
 */

#ifndef TOPIC_FRAME_BUTTON_H_
#define TOPIC_FRAME_BUTTON_H_

#include <stdint.h>
#include "../uORB.h"

typedef enum{
    SINGLE_CLICK = 0,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    LONG_KEYPRESS
}button_state;

struct frame_button_s
{
    uint64_t timestamp;
    button_state state;
};

/* register this as object request broker structure */
ORB_DECLARE(frame_button_state);


#endif
