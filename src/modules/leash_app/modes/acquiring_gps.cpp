#include "acquiring_gps.h"

#include <stdio.h>

#include "../displayhelper.h"
#include "../datamanager.h"

#include "main.h"
#include "menu.h"

//TODO[AK] These constants taken from position_estimator_inav, should by parametrized I guess
static const float min_eph_epv = 2.0f;	// min EPH/EPV, used for weight calculation
static const float max_eph_epv = 20.0f;	// max EPH/EPV acceptable for estimation

namespace modes
{

bool drone_status(DataManager *dm);

Acquiring_gps::Acquiring_gps()
    //:drone_has_gps(false)
    :drone_has_gps(true)
    ,drone_has_home(true)
    ,leash_has_gps(false)
    ,leash_has_home(false)
{
    DisplayHelper::showInfo(INFO_ACQUIRING_GPS_LEASH, 0);
}

int Acquiring_gps::getTimeout()
{
    return -1;
}

void Acquiring_gps::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
    awaitMask[FD_LocalPos] = 1;
    awaitMask[FD_VehicleStatus] = 1;
    awaitMask[FD_DroneRowGPS] = 1;
}

Base* Acquiring_gps::doEvent(int orbId)
{
    DataManager *dm = DataManager::instance();
    Base *nextMode = nullptr;

    if (drone_has_gps 
            && drone_has_home
            && leash_has_home 
            && leash_has_gps)
    {
        nextMode = new Main();
    }
    else if (leash_has_home && leash_has_gps)
    {
        DisplayHelper::showInfo(INFO_ACQUIRING_GPS_AIRDOG);
    }

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_MODE))
        {
            nextMode = new Menu();
        }
    }
    else if (orbId == FD_LocalPos)
    {
        if (dm->localPos.xy_valid)
        {
            leash_has_gps = true;
        }
    }
    else if (orbId == FD_VehicleStatus)
    {
        if (dm->vehicle_status.condition_home_position_valid)
        {
            leash_has_home = true;
        }
    }
    else if (orbId == FD_DroneRowGPS)
    {
        drone_has_gps = drone_status(dm);
    }

    // Check if we are in service screen
    Base* service = checkServiceScreen(orbId);
    if (service)
        nextMode = service;

    return nextMode;
}

bool drone_status(DataManager *dm)
{
    bool result = false;
    float eph = dm->droneRawGPS.eph;
    float epv = dm->droneRawGPS.epv;
    int sats  = dm->droneRawGPS.satellites_visible;

    if (eph < max_eph_epv * 0.7f && epv < max_eph_epv * 0.7f && sats >= 3) {
        result = true;
    }
    return result;
}

}
