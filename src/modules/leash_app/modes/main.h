#pragma once

#include "base.h"

#include <uORB/topics/leash_display.h>

//#include <time.h>
#include <drivers/drv_hrt.h>

namespace modes
{

class Main : public Base
{
public:
    Main();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);
private:
    enum MainStates
    {
        GROUNDED = 0, 
        IN_FLINGHT
    };
    enum SubStates
    {
        NONE = 0,
        // -- GROUNDED subs --
        HELP,
        CONFIRM_TAKEOFF,
        TAKEOFF_CONFIRMED,
        TAKEOFF_FAILED,
        // -- IN_FLIGHT subs --
        PLAY,
        PAUSE,
        // -- Landing and Taking off subs --
        TAKING_OFF,
        LANDING,
        RTL,
    };
    struct Condition
    {
        MainStates main;
        SubStates sub;
    };
    struct DisplayInfo
    {
        int airdog_mode;
        int follow_mode;
        int land_mode;
    };

	const hrt_abstime command_responce_time = 10000000;
	hrt_abstime local_timer = 0;

    struct DisplayInfo displayInfo;
    struct Condition baseCondition;
    Base* makeAction();
    Base* processGround(int orbId);
    Base* processTakeoff(int orbId);
    Base* processLandRTL(int orbId);
    Base* processHelp(int orbId);
    Base* processFlight(int orbId);
};

}
