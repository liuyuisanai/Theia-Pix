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
};

}
