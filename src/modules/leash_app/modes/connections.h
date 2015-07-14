#pragma once

#include "base.h"

namespace modes {

class Connections : public Base
{
    public:
        Connections();

        virtual int getTimeout();
        virtual void listenForEvents(bool awaitMask[]);
        virtual Base* doEvent(int orbId);
    private:
        enum class ModeState{
            UNKNOWN = 0,
            NOT_PAIRED,
            PAIRING,
            DISCONNECTED,
            CONNECTING,
            CONNECTED
        };

        ModeState CurrentState;

};

} //end of namespace modes
