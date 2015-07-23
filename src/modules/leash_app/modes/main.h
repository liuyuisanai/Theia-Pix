#pragma once

#include "base.h"

#include <uORB/topics/leash_display.h>

namespace modes
{

class Main : public Base
{
public:
    Main();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);
protected:
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
        TAKING_OFF,
    };
    struct Condition
    {
        MainStates main;
        SubStates sub;
    };

    struct Condition baseCondition;
    Base* makeAction();
};

}
