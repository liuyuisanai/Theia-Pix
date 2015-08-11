#include "error.h"

#include <stdio.h>

#include "../displayhelper.h"

namespace modes
{

int Error::lastErrorStamp = 0;

Error::Error() :
    isErrorShowed(false),
    lastErrorCode(0),
    lastErrorTime((time_t)0)
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
        if (DataManager::instance()->airdog_status.error_stamp != lastErrorStamp &&
                DataManager::instance()->airdog_status.error_code != 0)
        {
            lastErrorStamp = DataManager::instance()->airdog_status.error_stamp;
            lastErrorCode = DataManager::instance()->airdog_status.error_code;
            isErrorShowed = true;
            time(&lastErrorTime);

            if (!onError(lastErrorCode))
            {
                DisplayHelper::showInfo(INFO_ERROR, lastErrorCode);
            }
        }
    }
    else if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_OK) || key_pressed(BTN_BACK))
        {
            isErrorShowed = false;
        }
    }

    time_t now;
    time(&now);
    if (1000 * (now - lastErrorTime) > ERROR_SHOW_INTERVAL)
    {
        // timeout
        isErrorShowed = false;
    }
    return nullptr;
}

bool Error::onError(int errorCode)
{
    return false;
}

}
