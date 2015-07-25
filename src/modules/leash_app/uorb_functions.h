#pragma once

#include <drivers/drv_hrt.h>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/airdog_status.h>

extern void sendAirDogCommnad(enum VEHICLE_CMD command,
                             float param1 = 0,
                             float param2 = 0,
                             float param3 = 0,
                             float param4 = 0,
                             double param5 = 0,
                             double param6 = 0,
                             float param7 = 0);

extern void send_arm_command(const airdog_status_s &s);
