#include "main.h"

#include <uORB/topics/leash_display.h>
#include <uORB/topics/bt_state.h>
#include <uORB/topics/vehicle_status.h>

#include "menu.h"
#include "connect.h"
#include "../displayhelper.h"
#include "../uorb_functions.h"

namespace modes
{

Main::Main()
{
    baseCondition.main = GROUNDED;
    baseCondition.sub = NONE;

    DisplayHelper::showMain(MAINSCREEN_INFO, "zhopa",
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
                if (dm->airdog_status.state_aird < AIRD_STATE_TAKING_OFF) 
                {
                    baseCondition.sub = TAKEOFF_CONFIRMED;
                }
                else
                {
                    DisplayHelper::showInfo(INFO_TAKEOFF_FAILED, 0);
                    baseCondition.sub = TAKEOFF_FAILED;
                }
            }
            else if (baseCondition.sub == TAKEOFF_FAILED)
            {
                baseCondition.sub = NONE;
            }
            nextMode = makeAction();
        }
    }
    else if (orbId == FD_BLRHandler)
    {
        if (dm->bt_handler.global_state == CONNECTING)
        {
            nextMode = new ModeConnect(ModeState::DISCONNECTED);
        }
    }
    DOG_PRINT("[leash_app]{main screen} condition %d\n", baseCondition.sub);

    return nextMode;
}

Base* Main::makeAction()
{
    Base *nextMode = nullptr;

    if (baseCondition.main == GROUNDED)
    {
        switch (baseCondition.sub)
        {
            case NONE:
                DisplayHelper::showMain(MAINSCREEN_INFO, "zhopa",
                                        AIRDOGMODE_NONE, FOLLOW_PATH, LAND_SPOT);
            case CONFIRM_TAKEOFF:
                DOG_PRINT("[leash_app]{main menu} confirm zhopa screen\n");
                DisplayHelper::showMain(MAINSCREEN_CONFIRM_TAKEOFF, "zhopa", 0, 0, 0);
                break;
            case TAKEOFF_CONFIRMED:
                DOG_PRINT("[leash_app]{main menu} takeoff confirm\n");
                sendAirDogCommnad(VEHICLE_CMD_NAV_REMOTE_CMD,
                                     REMOTE_CMD_TAKEOFF,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     0);
                DisplayHelper::showMain(MAINSCREEN_TAKING_OFF, "zhopa", 0, 0, 0);
                baseCondition.sub = TAKING_OFF;
                break;
            case TAKING_OFF:
                DisplayHelper::showMain(MAINSCREEN_TAKING_OFF, "zhopa", 0, 0, 0);
                break;
            default:
                break;
        }
    }
    return nextMode;
}

}
