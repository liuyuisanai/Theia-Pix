#include "acquiring_gps.h"

#include <stdio.h>

#include "../displayhelper.h"
#include "../datamanager.h"

#include "main.h"
#include "menu.h"

namespace modes
{

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
}

Base* Acquiring_gps::doEvent(int orbId)
{
    DataManager *dm = DataManager::instance();
    Base *nextMode = nullptr;

    if (leash_has_gps 
            && drone_has_gps 
            && leash_has_home 
            && drone_has_home)
    {
        nextMode = new Main();
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
    //nextMode = new Main();
    return nextMode;
}

}
