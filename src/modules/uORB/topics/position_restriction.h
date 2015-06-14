/****************************************************************************
 *
 *   Copyright (C) 2014 AirDog Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted.
 *
 ****************************************************************************/

/**
 * @file position_restriction.h
 * Restriction for target position while executing on of the Follow modes or Auto modes
 *
 * @author Ilya Nevdah
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"


/**
 * Position restriction line in WGS84 coordinates.
 *
 * This is the line that consists of ponts to define "safe path" for vehicle to stay on while executing Follow modes
 */
struct position_restriction_line_s
{
	double   first[3];  /**< first point*/
    double   last[3];   /**< last point*/
};

struct position_restriction_s
{
	struct position_restriction_line_s line;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(position_restriction);
