/**
 * @file set_drone_parameter.h
 * Definition of the set drone parameter command uORB topic.
 */

#ifndef TOPIC_SET_DRONE_PARAM_H_
#define TOPIC_SET_DRONE_PARAM_H_

struct set_drone_param_s {
    float param_value; ///< Onboard parameter value
    uint8_t target_system; ///< System ID
    uint8_t target_component; ///< Component ID
    char param_id[16]; ///< Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
    uint8_t param_type; ///< Onboard parameter type: see the MAV_PARAM_TYPE enum for supported data types.
};

/* register this as object request broker structure */
ORB_DECLARE(set_drone_parameter);

#endif
