#pragma once

#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

/** Common structure for mavlink rx or tx stats */
struct mavlink_stats_s {
        unsigned int version;
	unsigned total_bytes;
	unsigned heartbeat_count;
	unsigned gpos_count;
	unsigned trajectory_count;
	unsigned combo_count;
	unsigned error_bytes;
};

/* register this as object request broker structure */
ORB_DECLARE(mavlink_receive_stats);
ORB_DECLARE(mavlink_transmit_stats);
