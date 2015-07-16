#pragma once

#include <uORB/uORB.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/leash_display.h>
#include <uORB/topics/system_power.h>

enum Orbs
{
    FD_LeashDisplay,
    FD_AirdogStatus,
    FD_SystemPower,
    FD_Size
};

class DataManager
{
public:
    DataManager();
    ~DataManager();

    bool awaitResult[FD_Size];
    bool awaitMask[FD_Size];

    struct airdog_status_s airdog_status;
    struct system_power_s system_power;
    struct leash_display_s leash_display;

    bool wait(int timeout);
    void clearAwaitMask();
protected:
    const struct orb_metadata *orbId[FD_Size];
    int fds[FD_Size];
    void *orbData[FD_Size];
};

