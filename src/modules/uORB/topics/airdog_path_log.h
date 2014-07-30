/**
 * @file airdog_path_log.h
 * Definition of the logger trigger command uORB topic.
 */

#ifndef TOPIC_PATH_LOG_H_
#define TOPIC_PATH_LOG_H_

#include <stdint.h>
#include "../uORB.h"

struct airdog_path_log_s {
	uint8_t start;
    uint8_t stop;
 }; /**< command sent to sdlog2 */

/* register this as object request broker structure */
ORB_DECLARE(airdog_path_log);

#endif
