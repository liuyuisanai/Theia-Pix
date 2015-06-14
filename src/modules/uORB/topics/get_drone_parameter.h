/**
 * @file set_drone_parameter.h
 * Definition of the set drone parameter command uORB topic.
 */


#ifndef TOPIC_GET_DRONE_PARAM_H_
#define TOPIC_GET_DRONE_PARAM_H_

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"

/**
* @addtogroup topics
* @{
*/

struct get_drone_param_s {
    int16_t param_index; ///< Parameter index. Send -1 to use the param ID field as identifier (else the param id will be ignored)
    uint8_t target_system; ///< System ID
    uint8_t target_component; ///< Component ID
    char param_id[16]; ///< Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
};

/**
* @}
*/

/* register this as object request broker structure */
ORB_DECLARE(get_drone_parameter);

#endif
