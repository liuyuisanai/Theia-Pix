#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"

/**
 * @addtogroup topics
 * @{
 */

 /**
 *
 */
struct external_trajectory_s {
	uint8_t point_type; /**< Indicates whether movement has crossed the threshold, 0 - still point, 1 - moving */
	uint8_t sysid; 		/**< External system id */

	// GPOS fields follow
	uint64_t timestamp;	/**< Time of this estimate, in microseconds since system start */
	double lat;			/**< Latitude in degrees */
	double lon;			/**< Longitude in degrees */
	// TODO! Check if AMSL or WGS84
	float alt;			/**< Altitude AMSL in meters */
	float relative_alt; /**< Altitude above ground in meters */
	float vel_n; 		/**< Ground north velocity, m/s	*/
	float vel_e;		/**< Ground east velocity, m/s */
	float vel_d;		/**< Ground downside velocity, m/s */
	float heading;   	/**< Compass heading in radians [0..2PI) */
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(external_trajectory);
