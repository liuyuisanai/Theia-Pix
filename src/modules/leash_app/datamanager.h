#pragma once

#include <uORB/uORB.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/bt21_laird.h>
#include <uORB/topics/bt_state.h>
#include <uORB/topics/calibrator.h>
#include <uORB/topics/kbd_handler.h>
#include <uORB/topics/leash_display.h>
#include <uORB/topics/mavlink_stats.h>
#include <uORB/topics/system_power.h>
#include <uORB/topics/target_global_position.h>
#include <uORB/topics/target_gps_raw.h>
#include <uORB/topics/vehicle_gps_position.h>
#include <uORB/topics/vehicle_local_position.h>
#include <uORB/topics/vehicle_status.h>

enum Orbs
{
    FD_AirdogStatus = 0,
    FD_BLRHandler,
    FD_Calibrator,
    FD_DroneRowGPS,
    FD_KbdHandler,
    FD_LeashRowGPS,
    FD_LocalPos,
    FD_DroneLocalPos,
    FD_MavlinkStatus,
    FD_BTLinkQuality,
    FD_SystemPower,
    FD_VehicleStatus,
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
    struct target_gps_raw_s droneRawGPS;
    struct bt_link_status_s  btLinkQuality;
    struct target_global_position_s  droneLocalPos;
    struct vehicle_gps_position_s leashRawGPS;
    struct vehicle_local_position_s localPos;
    struct vehicle_status_s vehicle_status;

    bool wait(int timeout);
    void clearAwait();
protected:
    static DataManager* _instance;
    const struct orb_metadata *orbId[FD_Size];
    int fds[FD_Size];
    void *orbData[FD_Size];
};

