#include "menu.h"

#include <stdio.h>

#include "main.h"
#include "../displayhelper.h"

namespace modes
{
struct Menu::Entry Menu::entries[Menu::MENUENTRY_SIZE] =
{
// -------- Top level menu
{
    // Menu::MENUENTRY_ACTIVITIES,
    MENUTYPE_ACTIVITIES,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_CUSTOMIZE,
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_SNOWBOARD,
    Menu::MENUENTRY_EXIT,
},
{
    // Menu::MENUENTRY_CUSTOMIZE,
    MENUTYPE_CUSTOMIZE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_ACTIVITIES,
    Menu::MENUENTRY_ALTITUDE,
    Menu::MENUENTRY_EXIT,
},
{
    // Menu::MENUENTRY_SETTINGS,
    MENUTYPE_SETTINGS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_ACTIVITIES,
    Menu::MENUENTRY_CUSTOMIZE,
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_EXIT,
},

// -------- Activities list
{
    // Menu::MENUENTRY_SNOWBOARD,
    MENUTYPE_SNOWBOARD,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    "Snowboarding",
    Menu::MENUENTRY_SURF,
    Menu::MENUENTRY_SURF,
    Menu::MENUENTRY_SELECT,
    Menu::MENUENTRY_ACTIVITIES,
},
{
    // Menu::MENUENTRY_SURF,
    MENUTYPE_SNOWBOARD,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    "Surf",
    Menu::MENUENTRY_SNOWBOARD,
    Menu::MENUENTRY_SNOWBOARD,
    Menu::MENUENTRY_SELECT,
    Menu::MENUENTRY_ACTIVITIES,
},

// -------- Activity menu
{
    // Menu::MENUENTRY_SELECT,
    MENUTYPE_SELECT,
    0,
    0,
    nullptr, // use previous preset name
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_ACTION,
    Menu::MENUENTRY_PREVIOUS,
},

// -------- Settings menu
{
    // Menu::MENUENTRY_PAIRING,
    MENUTYPE_PAIRING,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_CALIBRATION,
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_SETTINGS,
},
{
    // Menu::MENUENTRY_CALIBRATION,
    MENUTYPE_CALIBRATION,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_SETTINGS,
},
{
    // Menu::MENUENTRY_AIRDOG_CALIBRATION,
    MENUTYPE_CALIBRATION_AIRDOG,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_CALIBRATION,
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_SETTINGS,
},

// -------- Calibration menu
{
    // Menu::MENUENTRY_COMPASS,
    MENUTYPE_COMPASS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_ACCELS,
    Menu::MENUENTRY_GYRO,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_ACCELS,
    MENUTYPE_ACCELS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_GYRO,
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_GYRO,
    MENUTYPE_GYRO,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_COMPASS,
    Menu::MENUENTRY_ACCELS,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_PREVIOUS,
},

// -------- Customize menu
{
    // Menu::MENUENTRY_ALTITUDE,
    MENUTYPE_ALTITUDE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_FOLLOW,
    Menu::MENUENTRY_CANCEL,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_FOLLOW,
    MENUTYPE_FOLLOW,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_LAND,
    Menu::MENUENTRY_ALTITUDE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_LAND,
    MENUTYPE_LANDING,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_SAVE,
    Menu::MENUENTRY_FOLLOW,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_SAVE,
    MENUTYPE_SAVE,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_CANCEL,
    Menu::MENUENTRY_LAND,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
{
    // Menu::MENUENTRY_CANCEL,
    MENUTYPE_CANCEL,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_ALTITUDE,
    Menu::MENUENTRY_SAVE,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_CUSTOMIZE,
},
};

Menu::Menu()
{
    currentPresetName = nullptr;
    currentEntry = 0;
    previousEntry = -1;
    showEntry();
}

int Menu::getTimeout()
{
    return -1;
}

void Menu::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
}

Base* Menu::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    printf("buttons %x \n", DataManager::instance()->kbd_handler.buttons);

    if (key_pressed(BTN_OK))
    {
        if (entries[currentEntry].ok >= 0)
        {
            // save previous menu entry only when going to level down
            // skip left right movement
            previousEntry = currentEntry;
        }

        nextMode = switchEntry(entries[currentEntry].ok);
    }
    else if (key_pressed(BTN_BACK))
    {
        nextMode = switchEntry(entries[currentEntry].back);
    }
    else if (key_pressed(BTN_RIGHT))
    {
        nextMode = switchEntry(entries[currentEntry].next);
    }
    else if (key_pressed(BTN_LEFT))
    {
        nextMode = switchEntry(entries[currentEntry].prev);
    }

    return nextMode;
}

Base* Menu::makeAction()
{
    Base *nextMode = nullptr;

    switch (currentEntry)
    {
        case MENUENTRY_SELECT:
            nextMode = new Main();
            break;
    }

    return nextMode;
}

void Menu::showEntry()
{
    const char *presetName = entries[currentEntry].text;

    if (presetName == nullptr)
    {
        presetName = currentPresetName;
    }

    printf("presetName %s\n", presetName);
    DisplayHelper::showMenu(entries[currentEntry].menuButtons, entries[currentEntry].menuType,
                            entries[currentEntry].menuValue, presetName);
}

Base* Menu::switchEntry(int newEntry)
{
    Base *nextMode = nullptr;

    if (newEntry == MENUENTRY_PREVIOUS)
    {
        switchEntry(previousEntry);
    }
    else if (newEntry == MENUENTRY_ACTION)
    {
        nextMode = makeAction();
    }
    else if (newEntry == MENUENTRY_EXIT)
    {
        nextMode = new Main();
    }
    else if (newEntry < MENUENTRY_SIZE && newEntry >= 0)
    {
        currentEntry = newEntry;

        if (entries[currentEntry].text != nullptr)
        {
            currentPresetName = entries[currentEntry].text;
            printf("current preset %s\n", currentPresetName);
        }
        showEntry();
    }

    return nextMode;
}

}
