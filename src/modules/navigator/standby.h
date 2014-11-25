/***************************************************************************
 *
 *   Copyright (c) 2014 AirDog Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is NOT permitted!
 *
 ****************************************************************************/
/**
 * @file standby.h
 *
 * @author Ilya Nevdah <ilya@airdog.com>
 */

#ifndef NAVIGATOR_STANDBY_H
#define NAVIGATOR_STANDBY_H

#include <mathlib/mathlib.h>
#include <geo/geo.h>
#include <controllib/blocks.hpp>
#include <controllib/block/BlockParam.hpp>
#include <uORB/topics/vehicle_command.h>

#include "navigator_mode.h"
#include "mission_block.h"

class Standby : public MissionBlock
{
public:
	Standby(Navigator *navigator, const char *name);

	~Standby();

	virtual void on_inactive();

	virtual void on_activation();

	virtual void on_active();

	virtual void execute_vehicle_command();

private:

};

#endif
