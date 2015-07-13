#include "datamanager.hpp"

#include <poll.h>
#include <string.h>
#include <stdio.h>

DataManager::DataManager()
{
    // get orbs id
    orbId[FD_AirdogStatus] = ORB_ID(airdog_status);
    orbId[FD_SystemPower] = ORB_ID(system_power);
    orbId[FD_KbdHandler] = ORB_ID(kbd_handler);
    orbId[FD_LeashDisplay] = ORB_ID(leash_display);

    // listen orbs
    for (int i = 0; i < FD_Size; i++)
    {
        fds[i] = orb_subscribe(orbId[i]);
    }

    // set orbs interval
    orb_set_interval(fds[FD_AirdogStatus], 5000);
    orb_set_interval(fds[FD_SystemPower], 5000);

    // set addresses
    memset(orbData, 0, sizeof(orbData));

    orbData[FD_AirdogStatus] = &airdog_status;
    orbData[FD_SystemPower] = &system_power;
    orbData[FD_KbdHandler] = &kbd_handler;
    orbData[FD_LeashDisplay] = &leash_display;
}

DataManager::~DataManager()
{
    for (int i = 0; i < FD_Size; i++)
    {
        orb_unsubscribe(fds[i]);
    }
}

bool DataManager::wait(int timeout)
{
    int r = 0;
    struct pollfd pollfds[FD_Size];
    bool hasChanges = false;

    memset(awaitResult, 0, sizeof(awaitResult));

    // clean all events
    memset(pollfds, 0, sizeof(pollfds));

    for (int i = 0; i < FD_Size; i++)
    {
        pollfds[i].fd = fds[i];
        pollfds[i].events = POLLIN;
    }

    r = poll(pollfds, FD_Size, timeout);

    if (r == -1)
    {
        printf("poll failed\n");
    }

    for (int i = 0; i < FD_Size; i++)
    {
        if (pollfds[i].revents & POLLIN)
        {
            hasChanges = true;
            awaitResult[i] = true;
            orb_copy(orbId[i], fds[i], orbData[i]);
        }
    }

    return hasChanges;
}
