#pragma once

#include "../datamanager.h"
#include "../button_handler.h"

namespace modes
{

class Base
{
    public:
        virtual ~Base() {}

        virtual int getTimeout() = 0;
        virtual void listenForEvents(bool awaitMask[]) = 0;
        virtual Base* doEvent(int orbId) = 0;
};

}
