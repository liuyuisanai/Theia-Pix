#include "main.h"

#include <uORB/topics/leash_display.h>
#include <uORB/topics/bt_state.h>
#include <uORB/topics/vehicle_status.h>

#include "menu.h"
#include "connect.h"
#include "../displayhelper.h"
#include "../uorb_functions.hpp"

namespace modes
{

Main::Main()
{
    baseCondition.main = GROUNDED;
    baseCondition.sub = NONE;

    DisplayHelper::showMain(MAINSCREEN_INFO, "Snowboard",
                            AIRDOGMODE_NONE, FOLLOW_PATH, LAND_SPOT);
}

int Main::getTimeout()
{
    return -1;
}

void Main::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_AirdogStatus] = 1;
    awaitMask[FD_KbdHandler] = 1;
    awaitMask[FD_BLRHandler] = 1;
}

Base* Main::doEvent(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    if (orbId == FD_KbdHandler) 
    {
        if (key_pressed(BTN_MODE))
        {
            nextMode = new Menu();
        }
        else if (key_pressed(BTN_TO_H))
        {
            baseCondition.sub = CONFIRM_TAKEOFF;
            nextMode = makeAction();
        }
        else if (key_pressed(BTN_OK))
        {
            if (baseCondition.sub == CONFIRM_TAKEOFF)
            {
                if (dm->airdog_status.state_aird < AIRD_STATE_TAKING_OFF) {
                    baseCondition.sub = TAKEOFF_CONFIRMED;
                }
                nextMode = makeAction();
            }
        }
    }
    else if (orbId == FD_BLRHandler)
    {
        if (dm->bt_handler.global_state == CONNECTING)
        {
            nextMode = new ModeConnect(ModeState::DISCONNECTED);
        }
    }

    return nextMode;
}

Base* Main::makeAction()
{
    Base *nextMode = nullptr;
    DroneCommand *dc = DroneCommand::instance();

    if (baseCondition.main == GROUNDED)
    {
        switch (baseCondition.sub)
        {
            case CONFIRM_TAKEOFF:
                DisplayHelper::showMenu(MAINSCREEN_CONFIRM_TAKEOFF, 0, 0, 0);
                break;
            case TAKEOFF_CONFIRMED:
                if (airdog_status
                dc->
                break;
            default:
                break;
        }
    }
    return nextMode;
}

}
