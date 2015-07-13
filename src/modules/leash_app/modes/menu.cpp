#include "menu.h"

#include <stdio.h>

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
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_SETTINGS,
    Menu::MENUENTRY_SNOWBOARD,
    -1,
},
{
    // Menu::MENUENTRY_SETTINGS,
    MENUTYPE_SETTINGS,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    nullptr,
    Menu::MENUENTRY_ACTIVITIES,
    Menu::MENUENTRY_ACTIVITIES,
    -1,
    -1,
},
{
    // Menu::MENUENTRY_SNOWBOARD,
    MENUTYPE_SNOWBOARD,
    0,
    MENUBUTTON_LEFT | MENUBUTTON_RIGHT,
    "Snowboarding",
    Menu::MENUENTRY_SURF,
    Menu::MENUENTRY_SURF,
    -1,
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
    -1,
    Menu::MENUENTRY_ACTIVITIES,
}
};

Menu::Menu()
{
    currentEntry = 0;
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
    printf("buttons %x \n", DataManager::instance()->kbd_handler.buttons);

    if (DataManager::instance()->kbd_handler.buttons == 0x1)
    { // ok
        switchEntry(entries[currentEntry].ok);
    }
    else if (DataManager::instance()->kbd_handler.buttons == 0x10)
    { // back
        switchEntry(entries[currentEntry].back);
    }
    else if (DataManager::instance()->kbd_handler.buttons == 0x20)
    { // right
        switchEntry(entries[currentEntry].next);
    }
    else if (DataManager::instance()->kbd_handler.buttons == 0x100)
    { // left
        switchEntry(entries[currentEntry].prev);
    }

    return nullptr;
}

void Menu::showEntry()
{
    DisplayHelper::showMenu(entries[currentEntry].menuButtons, entries[currentEntry].menuType,
                            entries[currentEntry].menuValue, entries[currentEntry].presetName);
}

void Menu::switchEntry(int newEntry)
{
    if (newEntry != -1)
    {
        currentEntry = newEntry;
        showEntry();
    }
}

}
