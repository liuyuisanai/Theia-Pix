#pragma once

#include "base.h"

namespace modes
{

class Logo : public Base
{
    public:
        Logo();

        virtual int getTimeout();
        virtual void listenForEvents(bool awaitMask[]);
        virtual Base* doEvent(int orbId);
};

}
