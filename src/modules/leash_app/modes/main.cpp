#include "main.h"

#include <uORB/topics/leash_display.h>

#include "menu.h"
#include "../displayhelper.h"

namespace modes
{

Main::Main()
{
    DisplayHelper::showMain(MAINSCREEN_INFO, "Snowboard",
                            AIRDOGMODE_NONE, FOLLOW_PATH, LAND_SPOT);
}

int Main::getTimeout()
{
    return -1;
}

void Main::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
}

Base* Main::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (key_pressed(BTN_MODE))
    {
        nextMode = new Menu();
    }

    return nextMode;
}

}
