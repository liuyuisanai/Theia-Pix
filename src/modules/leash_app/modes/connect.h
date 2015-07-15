#pragma once

#include "base.h"

namespace modes {

class ModeConnect : public Base
{
    public:
        ModeConnect();
        virtual ~ModeConnect();

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

        // == methods ==
        void getConState();
        void BTPairing(bool start = 1);

};

} //end of namespace modes
