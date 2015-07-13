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
        MENUENTRY_ACTIVITIES,
        MENUENTRY_SETTINGS,
        MENUENTRY_SNOWBOARD,
        MENUENTRY_SURF,
        MENUENTRY_SIZE
    };

    struct Entry
    {
        int menuType;
        int menuValue;
        int menuButtons;
        char *presetName;
        int next;
        int prev;
        int ok;
        int back;
    };

    int currentEntry;
    static struct Entry entries[MENUENTRY_SIZE];

    void showEntry();
    void switchEntry(int newEntry);
};

}
