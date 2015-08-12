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
    awaitMask[FD_DroneLocalPos] = 1;
    awaitMask[FD_LeashRowGPS] = 1;
    awaitMask[FD_BTLinkQuality] = 1;
}

Base* Service::doEvent(int orbId)
{
    List::doEvent(orbId);
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();


    if (orbId == FD_DroneLocalPos
            || orbId == FD_LeashRowGPS
            || orbId == FD_BTLinkQuality)
    {
        char d_eph[100];
        char d_epv[100];
        char l_eph[100];
        char l_epv[100];
        char l_sat_num[100];

        char bt_link_q[100];

        char *data[] = {
                    bt_link_q
                    ,d_eph
                    ,d_epv
                    ,l_eph
                    ,l_epv
                    ,l_sat_num
        };

        sprintf(bt_link_q, "link qual %d%%", (int)((dm->btLinkQuality.link_quality) / 2.55));

        sprintf(l_eph, "L epH %.3f", (double) dm->leashRawGPS.eph);
        sprintf(l_epv, "L epV %.3f", (double) dm->leashRawGPS.epv);
        sprintf(l_sat_num, "L sat %d", dm->leashRawGPS.satellites_used);
        sprintf(d_eph, "D epH %.3f", (double) dm->droneLocalPos.eph);
        sprintf(d_epv, "D epV %.3f", (double) dm->droneLocalPos.epv);

        setList( (const char**)data, 6);
    }
    else if (orbId == FD_KbdHandler)
    {
        if (key_LongPressed(BTN_FUTURE))
                nextMode = new ModeConnect();
    }

    return nextMode;
}

int Service::getTimeout()
{
    return -1;
}

}
