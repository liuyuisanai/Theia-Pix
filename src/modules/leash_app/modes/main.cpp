#include "main.h"

#include <uORB/topics/leash_display.h>
#include <uORB/topics/bt_state.h>

#include "menu.h"
#include "connect.h"
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
    awaitMask[FD_BLRHandler] = 1;
}

Base* Main::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (orbId == FD_KbdHandler) 
    {
        if (key_pressed(BTN_MODE))
        {
            nextMode = new Menu();
        }
    }
    else if (orbId == FD_BLRHandler)
    {
        DataManager *dm = DataManager::instance();
        if (dm->bt_handler.global_state == CONNECTING)
        {
            nextMode = new ModeConnect(ModeState::DISCONNECTED);
        }
    }

    return nextMode;
}

}
