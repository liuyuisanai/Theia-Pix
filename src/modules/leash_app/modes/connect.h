#pragma once

#include "base.h"

namespace modes {

enum class ModeState{
    UNKNOWN = 0,
    NOT_PAIRED,
    PAIRING,
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class ModeConnect : public Base
{
    public:
        ModeConnect(ModeState current = ModeState::UNKNOWN);
        virtual ~ModeConnect();

        virtual int getTimeout();
        virtual void listenForEvents(bool awaitMask[]);
        virtual Base* doEvent(int orbId);
    private:

        ModeState currentState;

        // == methods ==
        void getConState();
        void BTPairing(bool start = 1);

};

} //end of namespace modes
