#include "error.h"

#include <stdio.h>

#include "../displayhelper.h"

namespace modes
{

int Error::lastErrorId = 0;

Error::Error() :
    isErrorShowed(false),
    lastErrorCode(0)
{
}

int Error::getTimeout()
{
    int timeout = -1;

    if (isErrorShowed)
    {
        timeout = ERROR_SHOW_INTERVAL;
    }

    return timeout;
}

void Error::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_AirdogStatus] = 1;
    if (isErrorShowed) {
        awaitMask[FD_KbdHandler] = 1;
    }
}

Base* Error::doEvent(int orbId)
{
    if (orbId == FD_AirdogStatus)
    {
        if (DataManager::instance()->airdog_status.error_id != lastErrorId)
        {
            lastErrorId = DataManager::instance()->airdog_status.error_id;
            lastErrorCode = DataManager::instance()->airdog_status.error_code;
            isErrorShowed = true;

            DisplayHelper::showInfo(INFO_ERROR, lastErrorCode);
        }
    }
    else if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_OK) || key_pressed(BTN_BACK))
        {
            isErrorShowed = false;
        }
    }
    else if (orbId == 0)
    {
        // timeout
        isErrorShowed = false;
    }
    return nullptr;
}

}
