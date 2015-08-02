#pragma once

#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

/** Common structure for mavlink rx or tx stats */
/** Be sure to update sdlog defines bellow if you change the structure */
/** TODO! [AK] Make an automatic script at least for sdlog types if not names */
struct mavlink_stats_s {
        unsigned int version;
	unsigned total_bytes;
	unsigned heartbeat_count;
	unsigned gpos_count;
	unsigned trajectory_count;
	unsigned combo_count;
	unsigned error_bytes;
};
#define MAVLINK_STATS_SDLOG_TYPES "IIIIIII"
#define MAVLINK_STATS_SDLOG_NAMES "Version,TotalBs,HrtCnt,GposCnt,TrajCnt,ComboCnt,ErrorBs"

/* register this as object request broker structure */
ORB_DECLARE(mavlink_receive_stats);
ORB_DECLARE(mavlink_transmit_stats);
