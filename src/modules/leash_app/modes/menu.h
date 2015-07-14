#pragma once

#include "base.h"

#include <uORB/topics/leash_display.h>

namespace modes
{

class Menu : public Base
{
public:
    Menu();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

protected:
    enum {
        // functional entries
        MENUENTRY_ACTION = - 4,
        MENUENTRY_PREVIOUS = -3,
        MENUENTRY_IGNORE = -2,
        MENUENTRY_EXIT = -1,
        // Menu entries

        // Top level menu
        MENUENTRY_ACTIVITIES,
        MENUENTRY_CUSTOMIZE,
        MENUENTRY_SETTINGS,

        // Activities list
        MENUENTRY_SNOWBOARD,
        MENUENTRY_SURF,

        // Activity menu
        MENUENTRY_SELECT,

        // Settings menu
        MENUENTRY_PAIRING,
        MENUENTRY_CALIBRATION,
        MENUENTRY_AIRDOG_CALIBRATION,

        // Calibration menu
        MENUENTRY_COMPASS,
        MENUENTRY_ACCELS,
        MENUENTRY_GYRO,

        // Customize menu
        MENUENTRY_ALTITUDE,
        MENUENTRY_FOLLOW,
        MENUENTRY_LAND,
        MENUENTRY_SAVE,
        MENUENTRY_CANCEL,

        // Total menu entries count
        MENUENTRY_SIZE
    };

    struct Entry
    {
        int menuType;
        int menuValue;
        int menuButtons;
        const char *text;
        int next;
        int prev;
        int ok;
        int back;
    };

    int currentEntry;
    int previousEntry;
    //TODO: get current preset from somewhere
    const char *currentPresetName;
    static struct Entry entries[MENUENTRY_SIZE];

    Base* makeAction();
    void showEntry();
    Base* switchEntry(int newEntry);
};

}
