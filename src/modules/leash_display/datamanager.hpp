#pragma once

#include <uORB/uORB.h>
#include <uORB/topics/kbd_handler.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/leash_display.h>
#include <uORB/topics/system_power.h>

enum Orbs
{
    FD_AirdogStatus = 0,
    FD_SystemPower,
    FD_KbdHandler,
    FD_LeashDisplay,
    FD_Size
};

class DataManager
{
public:
    DataManager();
    ~DataManager();

    bool awaitResult[FD_Size];

    struct airdog_status_s airdog_status;
    struct system_power_s system_power;
    struct kbd_handler_s kbd_handler;
    struct leash_display_s leash_display;

    bool wait(int timeout);
protected:
    const struct orb_metadata *orbId[FD_Size];
    int fds[FD_Size];
    void *orbData[FD_Size];
};

