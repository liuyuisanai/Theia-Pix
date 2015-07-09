#pragma once

#include <time.h>

class Status {
public:
    int screenId;
    uint8_t airdog_battery;
    float leash_battery;

    int mode;
    int buttons;

    Status();
    ~Status();

    bool update();

protected:
    enum PollFds
    {
        FD_AirdogStatus = 0,
        FD_LeashDisplay,
        FD_SystemPower,
        FD_KbdHandler,
        FD_Size
    };

    struct FdWait {
        int fd;
        int timeout;
        int lastTime;
    } fdsInfo[FD_Size];

    void updateAirdogBattery();
    void updateLeashDisplay();
    void updateLeashBattery();
};

