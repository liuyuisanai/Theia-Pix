#pragma once

#include <uORB/uORB.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/bt_state.h>
#include <uORB/topics/calibrator.h>
#include <uORB/topics/kbd_handler.h>
#include <uORB/topics/mavlink_stats.h>
#include <uORB/topics/system_power.h>
#include <uORB/topics/vehicle_local_position.h>
#include <uORB/topics/vehicle_status.h>

enum Orbs
{
    FD_AirdogStatus = 0,
    FD_VehicleStatus,
    FD_SystemPower,
    FD_KbdHandler,
    FD_BLRHandler,
    FD_Calibrator,
    FD_LocalPos,
    FD_MavlinkStatus,
    FD_Size
};

class DataManager
{
public:
    DataManager();
    ~DataManager();

    static DataManager* instance();
    bool awaitMask[FD_Size];
    bool awaitResult[FD_Size];

    struct airdog_status_s airdog_status;
    struct bt_state_s bt_handler;
    struct calibrator_s calibrator;
    struct kbd_handler_s kbd_handler;
    struct mavlink_stats_s mavlink_received_stats;
    struct system_power_s system_power;
    struct vehicle_status_s vehicle_status;
    struct vehicle_local_position_s localPos;

    bool wait(int timeout);
    void clearAwait();
protected:
    static DataManager* _instance;
    const struct orb_metadata *orbId[FD_Size];
    int fds[FD_Size];
    void *orbData[FD_Size];
};

