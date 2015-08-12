#include "base.h"
#include "service.h"

namespace modes
{

Base* Base::checkServiceScreen(int orbId)
{
    Base *nextMode = nullptr;
    if (orbId == FD_KbdHandler)
    {
        if (key_LongPressed(BTN_FUTURE))
        nextMode = new Service();
    }
    return nextMode;
}

}
