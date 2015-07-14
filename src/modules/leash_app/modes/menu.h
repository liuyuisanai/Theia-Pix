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
        MENUENTRY_PREVIOUS = -3,
        MENUENTRY_IGNORE = -2,
        MENUENTRY_EXIT = -1,
        // normal entries
        MENUENTRY_ACTIVITIES,
        MENUENTRY_CUSTOMIZE,
        MENUENTRY_SETTINGS,
        MENUENTRY_SNOWBOARD,
        MENUENTRY_SURF,
        MENUENTRY_SELECT,
        MENUENTRY_PAIRING,
        MENUENTRY_CALIBRATION,
        MENUENTRY_AIRDOG_CALIBRATION,
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

    void showEntry();
    void switchEntry(int newEntry);
};

}
