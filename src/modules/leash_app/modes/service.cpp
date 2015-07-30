#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "service.h"

namespace modes
{

Service::Service()
{
}

Service::~Service()
{
}

void Service::listenForEvents(bool awaitMask[])
{
    List::listenForEvents(awaitMask);
    awaitMask[FD_DroneRowGPS] = 1;
    awaitMask[FD_LeashRowGPS] = 1;
}

Base* Service::doEvent(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();


    if (orbId == FD_DroneRowGPS
            || orbId == FD_LeashRowGPS)
    {
        char d_eph[100];
        char d_epv[100];
        char d_sat_num[100];
        char l_eph[100];
        char l_epv[100];
        char l_sat_num[100];

        char *data[6] = {
                    d_eph
                    ,d_epv
                    ,d_sat_num
                    ,l_eph
                    ,l_epv
                    ,l_sat_num
        };

        sprintf(l_eph, "Leash eph %.3f", (double) dm->leashRawGPS.eph);
        sprintf(l_epv, "Leash epv %.3f", (double) dm->leashRawGPS.epv);
        sprintf(l_sat_num, "Leash satelites %d", dm->leashRawGPS.satellites_used);
        sprintf(d_eph, "Drone eph %.3f", (double) dm->droneRawGPS.eph);
        sprintf(d_epv, "Drone epv %.3f", (double) dm->droneRawGPS.epv);
        sprintf(d_sat_num, "Drone satelites %d", dm->droneRawGPS.satellites_visible);

        setList( (const char**)data, 6);
    }
    else if (orbId == FD_KbdHandler)
    {
        if (key_LongPressed(BTN_FUTURE))
        {
            nextMode = new Main();
        }
    }

    return nextMode;
}

int Service::getTimeout()
{
    return -1;
}

}
