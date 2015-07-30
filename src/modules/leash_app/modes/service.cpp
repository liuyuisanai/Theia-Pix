#include <stdio.h>
#include <stdlib.h>

#include "connect.h"
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
    List::doEvent(orbId);
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

        sprintf(l_eph, "L epH %.3f", (double) dm->leashRawGPS.eph);
        sprintf(l_epv, "L epV %.3f", (double) dm->leashRawGPS.epv);
        sprintf(l_sat_num, "L sat %d", dm->leashRawGPS.satellites_used);
        sprintf(d_eph, "D epH %.3f", (double) dm->droneRawGPS.eph);
        sprintf(d_epv, "D epV %.3f", (double) dm->droneRawGPS.epv);
        sprintf(d_sat_num, "D sat %d", dm->droneRawGPS.satellites_visible);

        setList( (const char**)data, 6);
    }
    else if (orbId == FD_KbdHandler)
    {
        if (dm->kbd_handler.buttons == 0)
        {
            switch(dm->kbd_handler.currentMode)
            {
                case (int)ModeId::FLIGHT:
                case (int)ModeId::FLIGHT_ALT:
                    nextMode = new Main();
                    break;
                default:
                    nextMode = new ModeConnect();
                    break;
            }
        }
    }

    return nextMode;
}

int Service::getTimeout()
{
    return -1;
}

}
