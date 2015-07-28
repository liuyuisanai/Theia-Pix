#pragma once

#include "base.h"

namespace modes
{

class Acquiring_gps : public Base
{
    public:
        Acquiring_gps();

        virtual int getTimeout();
        virtual void listenForEvents(bool awaitMask[]);
        virtual Base* doEvent(int orbId);
    private:
        bool drone_has_gps;
        bool drone_has_home;
        bool leash_has_gps;
        bool leash_has_home;
};

}
