/****************************************************************************
 *
 *   Copyright (C) 2012 - 2014 AirDog Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted provided that the following conditions
*/
/**
 * @file commander_request.h
 * Definition of the commander_request uORB topic.
 *
 * Commander_requests are requests to commander for vehicle status changes.
 *
 * Commander is the responsible app for managing system related tasks
 * and setting vehicle statuses other apps must make request for commander
 * if system related task or system state change is needed.
 *
 * Commander will process this request and decide what to do with it.
 *
 * @author Martins Frolovs <martins.f@airdog.com>
 */

#ifndef COMMANDER_REQUEST_H_
#define COMMANDER_REQUEST_H_

#include <stdint.h>
#include <stdbool.h>
#include "../uORB.h"
#include "vehicle_status.h"

/**
 * @addtogroup topics @{
 */

/**
 * Request type.
 */
typedef enum {
	V_MAIN_STATE_CHANGE = 0,    // Request to main_state change
	V_DISARM,                   // Request to disarm vehicle
    AIRD_STATE_CHANGE,           // Request to change airdog_state
    V_RESET_MODE_ARGS,			// Request to reset mode argumens 
    OTHER,						// Other request
} request_type_t;

/**
 * Camera mode
 */
typedef enum {
	UNDEFINED = 0,
	HORIZONTAL,
	AIM_TO_TARGET,
    LOOK_DOWN,
} camera_mode_t;


struct commander_request_s {
	request_type_t request_type;
	main_state_t main_state;
    airdog_state_t airdog_state;
    camera_mode_t camera_mode;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(commander_request);

#endif
