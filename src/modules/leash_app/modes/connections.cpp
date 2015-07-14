#include "connections.h"

namespace modes
{

Connections::Connections()
{
    CurrentState = UNKNOWN; 
}

void Connections::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
    awaitMask[FD_BLRHandler] = 1;
}

} //end of namespace modes
