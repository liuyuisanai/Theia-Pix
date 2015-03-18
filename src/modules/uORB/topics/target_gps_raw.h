/**
 * @file target_gps_raw.h
 * Raw GPS data from target uORB topic.
 */

#ifndef TARGET_GPS_RAW_T_H_
#define TARGET_GPS_RAW_T_H_

#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

/**
* Target GPS raw data.
*/
struct target_gps_raw_s {
	uint64_t timestamp_local;
	uint64_t timestamp_remote;
	uint8_t fix_type;
	int32_t lat;
	int32_t lon;
	int32_t alt;
	float eph;
	float epv;
	float vel;
	float cog_rad;
	uint8_t satellites_visible;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(target_gps_raw);

#endif
