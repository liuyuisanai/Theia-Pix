#include "menu.h"

#include <stdio.h>

#include "main.h"
#include "../displayhelper.h"

namespace modes
{
struct Menu::Entry Menu::entries[Menu::MENUENTRY_SIZE] =
{
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
    -1,
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
{
    // Menu::MENUENTRY_SELECT,
    MENUTYPE_SELECT,
    0,
    0,
    nullptr, // use previous preset name
    -1,
    -1,
    -1,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_PAIRING,
    MENUTYPE_PAIRING,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_CALIBRATION,
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_IGNORE,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_CALIBRATION,
    MENUTYPE_CALIBRATION,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_AIRDOG_CALIBRATION,
    Menu::MENUENTRY_PAIRING,
    -1,
    Menu::MENUENTRY_PREVIOUS,
},
{
    // Menu::MENUENTRY_AIRDOG_CALIBRATION,
    MENUTYPE_CALIBRATION_AIRDOG,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr, // use previous preset name
    Menu::MENUENTRY_PAIRING,
    Menu::MENUENTRY_CALIBRATION,
    -1,
    Menu::MENUENTRY_PREVIOUS,
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

        switchEntry(entries[currentEntry].ok);
    }
    else if (key_pressed(BTN_BACK))
    {
        switchEntry(entries[currentEntry].back);
        if (currentEntry == MENUENTRY_EXIT)
        {
            nextMode = new Main();
        }
    }
    else if (key_pressed(BTN_RIGHT))
    {
        switchEntry(entries[currentEntry].next);
    }
    else if (key_pressed(BTN_LEFT))
    {
        switchEntry(entries[currentEntry].prev);
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

void Menu::switchEntry(int newEntry)
{
    if (newEntry == MENUENTRY_PREVIOUS)
    {
        switchEntry(previousEntry);
    }
    else if (newEntry == MENUENTRY_IGNORE)
    {

    }
    else if (newEntry == MENUENTRY_EXIT)
    {
        currentEntry = newEntry;
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
}

}
