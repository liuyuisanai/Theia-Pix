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

    displayInfo.airdog_mode = AIRDOGMODE_NONE;
    /*TODO[Max] presset info for next two required*/
    displayInfo.follow_mode = FOLLOW_PATH;
    displayInfo.land_mode = LAND_SPOT;

    local_timer = 0;
    baseCondition.main = GROUNDED;
    baseCondition.sub = NONE;

    DisplayHelper::showMain(MAINSCREEN_INFO, "airdog"
                                ,displayInfo.airdog_mode
                                ,displayInfo.follow_mode
                                ,displayInfo.land_mode
                                );
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

Base* Main::processGround(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_MODE))
        {
            nextMode = new Menu();
        }
        else if (key_pressed(BTN_PLAY))
        {
            baseCondition.sub = CONFIRM_TAKEOFF;
            nextMode = makeAction();
        }
        else
        {
            baseCondition.sub = HELP;
            nextMode = makeAction();
        }
    }
    else if (orbId == FD_AirdogStatus)
    {
        if (dm->airdog_status.state_aird > AIRD_STATE_LANDED)
        {
            DOG_PRINT("[leash_app]{main} Detected flying drone while \
                    in GROUNDED main state. Leash restart?\n");
            baseCondition.main = IN_FLINGHT;
            baseCondition.sub = NONE;
            nextMode = makeAction();
        }
    }
    return nextMode;
}

Base* Main::processTakeoff(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    if (local_timer != 0)
    {
        if (local_timer + command_responce_time < hrt_absolute_time() )
        {
            if(dm->airdog_status.state_aird < AIRD_STATE_TAKING_OFF)
            {
                baseCondition.main = GROUNDED;
                baseCondition.sub = TAKEOFF_FAILED;
            }
            else
            {
                baseCondition.main = IN_FLINGHT;
                baseCondition.sub = TAKING_OFF;
            }
            local_timer = 0;
        }
    }
    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_MODE))
        {
            if (baseCondition.sub != TAKING_OFF)
            baseCondition.sub = NONE;
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
        }
    }
    else if (orbId == FD_AirdogStatus)
    {
        if (dm->airdog_status.state_aird == AIRD_STATE_TAKING_OFF)
        {
            baseCondition.main = IN_FLINGHT;
            baseCondition.sub = TAKING_OFF;
        }
        else if (dm->airdog_status.state_aird == AIRD_STATE_IN_AIR)
        {
            baseCondition.main = IN_FLINGHT;
            baseCondition.sub = NONE;
        }
    }
    nextMode = makeAction();
    return nextMode;
}

Base* Main::processHelp(int orbId)
{
    Base *nextMode = nullptr;
    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_MODE))
        {
            baseCondition.sub = NONE;
        }
        else if (key_pressed(BTN_PLAY))
        {
            baseCondition.sub = CONFIRM_TAKEOFF;
        }
    }
    nextMode = makeAction();
    return nextMode;
}

Base* Main::doEvent(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    /* -- disconnected -- */
    if (dm->bt_handler.global_state == CONNECTING)
    {
        nextMode = new ModeConnect(ModeState::DISCONNECTED);
    }
    else
    {
    /* -- grounded -- */
    if (baseCondition.main == GROUNDED)
    {
        switch (baseCondition.sub)
        {
            case NONE:
                nextMode = processGround(orbId);
                break;
            case HELP:
                nextMode = processHelp(orbId);
                break;
            case TAKEOFF_FAILED:
            case TAKEOFF_CONFIRMED:
            case CONFIRM_TAKEOFF:
                nextMode = processTakeoff(orbId);
                break;
            default:
                DOG_PRINT("[leash_app]{main} unexpected sub condition, got %d\n", baseCondition.sub);
                break;
        }
    }
    /* -- in air -- */
    else
    {
        switch(baseCondition.sub)
        {
            case TAKEOFF_CONFIRMED:
            case TAKING_OFF:
                nextMode = processTakeoff(orbId);
                break;
            case NONE:
            case PLAY:
            case PAUSE:
                nextMode = processFlight(orbId);
                break;
            case LANDING:
            case RTL:
                nextMode = processLandRTL(orbId);
                break;
            default:
                DOG_PRINT("[leash_app]{main} unexpected sub condition, got %d\n", baseCondition.sub);
                break;
        }
    }
    }
    DOG_PRINT("[leash_app]{main screen} condition %d.%d\n", baseCondition.main, baseCondition.sub);

    return nextMode;
}

Base* Main::makeAction()
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    if (baseCondition.main == GROUNDED)
    {
        switch (baseCondition.sub)
        {
            case NONE:
                DisplayHelper::showMain(MAINSCREEN_INFO, "airdog",
                                        AIRDOGMODE_NONE, FOLLOW_PATH, LAND_SPOT);
                break;
            case HELP:
                DisplayHelper::showMain(MAINSCREEN_READY_TO_TAKEOFF, "airdog",
                                        AIRDOGMODE_NONE, FOLLOW_PATH, LAND_SPOT);
                break;
            case CONFIRM_TAKEOFF:
                DOG_PRINT("[leash_app]{main menu} confirm airdog screen\n");
                DisplayHelper::showMain(MAINSCREEN_CONFIRM_TAKEOFF, "airdog", 0, 0, 0);
                break;
            case TAKEOFF_CONFIRMED:
                DOG_PRINT("[leash_app]{main menu} takeoff confirm\n");
                DisplayHelper::showMain(MAINSCREEN_TAKING_OFF, "airdog", 0, 0, 0);
                if (local_timer == 0)
                {
                    send_arm_command(dm->airdog_status);
                    local_timer = hrt_absolute_time();
                }
                break;
            case TAKING_OFF:
                DisplayHelper::showMain(MAINSCREEN_TAKING_OFF, "airdog", 0, 0, 0);
                break;
            case TAKEOFF_FAILED:
                DisplayHelper::showInfo(INFO_TAKEOFF_FAILED, 0);
                break;
            default:
                DOG_PRINT("[leash_app]{main} unexpected sub condition, got %d\n", baseCondition.sub);
                break;
        }
    }
    else
    {
        switch (baseCondition.sub)
        {
            case NONE:
            case PAUSE:
            case PLAY:
                DisplayHelper::showMain(MAINSCREEN_INFO, "airdog"
                                ,displayInfo.airdog_mode
                                ,displayInfo.follow_mode
                                ,displayInfo.land_mode
                                );
                break;
            case TAKING_OFF:
                DisplayHelper::showMain(MAINSCREEN_TAKING_OFF, "airdog", 0, 0, 0);
                break;
            case LANDING:
                DisplayHelper::showMain(MAINSCREEN_LANDING, "airdog", 0, 0, 0);
                break;
            case RTL:
                DisplayHelper::showMain(MAINSCREEN_GOING_HOME, "airdog", 0, 0, 0);
                break;
            default:
                DOG_PRINT("[leash_app]{main} unexpected sub condition, got %d\n", baseCondition.sub);
                break;
        }
    }
    return nextMode;
}

Base* Main::processFlight(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    /* Now all keyboard actions are processed in 'kbd_handler' in 'leash' module
     * moving them to this module should be considered as TODO[Max]
     * for now we are only monitoring airdog states to show corresponding info on 'leash_display'
     */
    
    if (orbId == FD_AirdogStatus || baseCondition.sub == NONE)
    {
        // process main state
        if (dm->airdog_status.state_main == MAIN_STATE_LOITER)
        {
            displayInfo.airdog_mode = AIRDOGMODE_PAUSE;
            baseCondition.sub = PAUSE;
        }
        else if (dm->airdog_status.state_main == MAIN_STATE_ABS_FOLLOW)
        {
            displayInfo.follow_mode = FOLLOW_ABS;
            displayInfo.airdog_mode = AIRDOGMODE_PLAY;
            baseCondition.sub = PLAY;
        }
        else if (dm->airdog_status.state_main == MAIN_STATE_AUTO_PATH_FOLLOW)
        {
            displayInfo.follow_mode = FOLLOW_PATH;
            displayInfo.airdog_mode = AIRDOGMODE_PLAY;
            baseCondition.sub = PLAY;
        }
        else if (dm->airdog_status.state_main == MAIN_STATE_RTL)
        {
            displayInfo.airdog_mode = AIRDOGMODE_PAUSE;
            baseCondition.sub = RTL;
        }
        // process airdog state
        if (dm->airdog_status.state_aird == AIRD_STATE_LANDING)
        {
            displayInfo.airdog_mode = AIRDOGMODE_PAUSE;
            baseCondition.sub = LANDING;
        }
    }
    nextMode = makeAction();
    return nextMode;
}

Base* Main::processLandRTL(int orbId)
{
    Base *nextMode = nullptr;
    DataManager *dm = DataManager::instance();

    if (orbId == FD_AirdogStatus)
    {
        if (dm->airdog_status.state_aird == AIRD_STATE_LANDING)
        {
            displayInfo.airdog_mode = AIRDOGMODE_PAUSE;
            baseCondition.sub = LANDING;
        }
        else if (dm->airdog_status.state_aird < AIRD_STATE_TAKING_OFF)
        {
            baseCondition.main = GROUNDED;
            baseCondition.sub = NONE;
        }
        else if (dm->airdog_status.state_main == MAIN_STATE_RTL)
        {
            displayInfo.airdog_mode = AIRDOGMODE_PAUSE;
            baseCondition.sub = RTL;
        }
    }
    nextMode = makeAction();
    return nextMode;
}

} //end of namespace modes
