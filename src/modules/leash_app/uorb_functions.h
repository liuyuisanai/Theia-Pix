#pragma once

#include <drivers/drv_hrt.h>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_command.h>

extern void sendAirDogCommnad(enum VEHICLE_CMD command,
                             float param1 = 0,
                             float param2 = 0,
                             float param3 = 0,
                             float param4 = 0,
                             double param5 = 0,
                             double param6 = 0,
                             float param7 = 0);
