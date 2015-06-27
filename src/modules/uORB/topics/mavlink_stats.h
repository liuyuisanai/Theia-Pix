#pragma once

#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

/** global 'actuator output is live' control. */
struct mavlink_receive_stats_s {

	unsigned total_bytes;
	unsigned heartbeat_count;
	unsigned gpos_count;
	unsigned trajectory_count;
	unsigned combo_count;
	unsigned error_bytes;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(mavlink_receive_stats);
