/**
 * @file set_drone_parameter.h
 * Definition of the set drone parameter command uORB topic.
 */

#ifndef TOPIC_PASS_DRONE_PARAM_H_
#define TOPIC_PASS_DRONE_PARAM_H_

#include <stdint.h>
#include "../uORB.h"


/**
* @addtogroup topics
* @{
*/

struct pass_drone_param_s {
    float param_value; ///< Onboard parameter value
    char param_id[16]; ///< Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
    uint8_t param_type;
};

/**
* @}
*/

/* register this as object request broker structure */
ORB_DECLARE(pass_drone_parameter);

#endif
