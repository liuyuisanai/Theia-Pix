#include "status.hpp"

#include <poll.h>
#include <string.h>
#include <stdio.h>

#include <uORB/uORB.h>
#include <uORB/topics/kbd_handler.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/leash_display.h>
#include <uORB/topics/system_power.h>

Status::Status()
{
    screenId = LEASHDISPLAY_NONE;
    airdog_battery = 0;
    leash_battery = 0;

    memset(fdsInfo, 0, sizeof(fdsInfo));

    fdsInfo[FD_AirdogStatus].fd = orb_subscribe(ORB_ID(airdog_status));
    fdsInfo[FD_SystemPower].fd= orb_subscribe(ORB_ID(system_power));
    fdsInfo[FD_LeashDisplay].fd = orb_subscribe(ORB_ID(leash_display));
    fdsInfo[FD_KbdHandler].fd = orb_subscribe(ORB_ID(kbd_handler));

    fdsInfo[FD_AirdogStatus].timeout = 5;
    fdsInfo[FD_SystemPower].timeout = 5;
    fdsInfo[FD_LeashDisplay].timeout = -1;
    fdsInfo[FD_KbdHandler].timeout = -1;
}

Status::~Status()
{
    for (int i = 0; i < FD_Size; i++)
    {
        orb_unsubscribe(fdsInfo[i].fd);
    }
}

bool Status::update()
{
    time_t t;
    int now;
    int timeout = -1;
    struct pollfd fds[FD_Size];
    int r = 0;
    bool hasChanges = false;

    time(&t);
    now = (int)t;

    // clean all events
    memset(fds, 0, sizeof(fds));

    for (int i = 0; i < FD_Size; i++)
    {
        fds[i].fd = fdsInfo[i].fd;

        // how much time gone from last call
        int d = now - fdsInfo[i].lastTime;

        // how much time you need to wait before check this value again
        // if negative or zero then you should check it
        int left = fdsInfo[i].timeout - d;

        if (left <= 0)
        {
            fds[i].events = POLLIN;
        }
        else if (timeout == -1 || left < timeout)
        {
            timeout = left;
        }
    }

    if (timeout != -1)
    {
        timeout *= 1000; // convert to miliseconds
    }

    r = poll(fds, FD_Size, timeout);
    if (r == -1)
    {
        printf("poll failed\n");
    }

    for (int i = 0; i < FD_Size; i++)
    {
        if (fds[i].revents & POLLIN)
        {
            hasChanges = true;
            fdsInfo[i].lastTime = now;
        }

    }

    if (fds[FD_AirdogStatus].revents & POLLIN)
    {
        printf("read airdog battery\n");
        updateAirdogBattery();
    }

    if (fds[FD_SystemPower].revents & POLLIN)
    {
        printf("read leash battery\n");
        updateLeashBattery();
    }

    if (fds[FD_LeashDisplay].revents & POLLIN)
    {
        printf("read leash display\n");
        updateLeashDisplay();
    }

    if (fds[FD_KbdHandler].revents & POLLIN)
    {
        printf("read keyboard\n");
        struct kbd_handler_s kh;

        orb_copy(ORB_ID(kbd_handler), fdsInfo[FD_KbdHandler].fd, &kh);
        mode = kh.currentMode;
        buttons = kh.buttons;
    }

    return hasChanges;
}

void Status::updateAirdogBattery()
{
    struct airdog_status_s airdog_status;

    orb_copy(ORB_ID(airdog_status), fdsInfo[FD_AirdogStatus].fd, &airdog_status);
    airdog_battery = airdog_status.battery_remaining;
}

void Status::updateLeashBattery()
{
    struct system_power_s system_power;

    orb_copy(ORB_ID(system_power), fdsInfo[FD_SystemPower].fd, &system_power);
    leash_battery = system_power.voltage5V_v;
}

void Status::updateLeashDisplay()
{
    struct leash_display_s leash_display;

    orb_copy(ORB_ID(leash_display), fdsInfo[FD_LeashDisplay].fd, &leash_display);
    screenId = leash_display.screenId;
}
