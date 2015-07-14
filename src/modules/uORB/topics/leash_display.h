/**
 * @file leash_display.h
 *
 * Definition of the leash display topic.
 */

#ifndef LEASH_DISPLAY_H_
#define LEASH_DISPLAY_H_

#include "../uORB.h"
#include <stdint.h>

/**
 * @addtogroup topics
 * @{
 */

enum
{
    LEASHDISPLAY_NONE,
    LEASHDISPLAY_LOGO,
    LEASHDISPLAY_MAIN,
    LEASHDISPLAY_MENU,
    LEASHDISPLAY_INFO,
};

enum
{
    FOLLOW_PATH,
    FOLLOW_ABS,
};

enum
{
    LAND_HOME,
    LAND_SPOT,
};

enum
{
    MAINSCREEN_INFO,
    MAINSCREEN_LANDING,
    MAINSCREEN_TAKING_OFF,
    MAINSCREEN_READY_TO_TAKEOFF,
    MAINSCREEN_CONFIRM_TAKEOFF,
    MAINSCREEN_GOING_HOME,
    MAINSCREEN_MAX
};

enum
{
    AIRDOGMODE_NONE,
    AIRDOGMODE_PLAY,
    AIRDOGMODE_PAUSE,
};

enum
{
    MENUTYPE_SETTINGS,
    MENUTYPE_ACTIVITIES,
    MENUTYPE_SNOWBOARD,
    MENUTYPE_PAIRING,
    MENUTYPE_CALIBRATION,
    MENUTYPE_CALIBRATION_AIRDOG,
    MENUTYPE_COMPASS,
    MENUTYPE_ACCELS,
    MENUTYPE_GYRO,
    MENUTYPE_SELECT,
    MENUTYPE_CUSTOMIZE,
    MENUTYPE_ALTITUDE,
    MENUTYPE_FOLLOW,
    MENUTYPE_LANDING,
    MENUTYPE_SAVE,
    MENUTYPE_CANCEL,
    MENUTYPE_MAX
};

enum
{
    INFO_CONNECTING_TO_AIRDOG,
    INFO_CONNECTION_LOST,
    INFO_TAKEOFF_FAILED,
    INFO_CALIBRATING_SENSORS,
    INFO_CALIBRATING_AIRDOG,
    INFO_PAIRING,
    INFO_ACQUIRING_GPS_LEASH,
    INFO_ACQUIRING_GPS_AIRDOG,
    INFO_CALIBRATING_HOLD_STILL,
    INFO_SUCCESS,
    INFO_FAILED,
    INFO_CALIBRATING_DANCE,
    INFO_NEXT_SIDE_UP,
    INFO_MAX,
};

enum
{
    MENUBUTTON_LEFT = 1,
    MENUBUTTON_RIGHT = 2
};

/**
 * leash display status
 */
struct leash_display_s {
    int screenId;
    char presetName[20];
    int mainMode;
    int followMode;
    int landMode;
    int airdogMode;
    int menuType;
    int menuValue;
    int menuButtons;
};

/**
 * @}
 */

/* register this as object request broker structure */
ORB_DECLARE(leash_display);

#endif
