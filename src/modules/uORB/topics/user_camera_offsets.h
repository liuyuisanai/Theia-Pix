/****************************************************************************
 *
 *   Copyright (C) 2014 AirDog Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted.
 *
 ****************************************************************************/

/**
 * @user_camera_offsets.h
 * Offsets setted by user from the leash, used in all modes to adjust camera pitch and yaw
 *
 * @author Max Shvetsov <max@airdog.com>
 */

#pragma once
#include "../uORB.h"

struct camera_user_offsets_s {
    int pitch_offset;
    int yaw_offset;
};

/* register this as object request broker structure */
ORB_DECLARE(camera_user_offsets);
