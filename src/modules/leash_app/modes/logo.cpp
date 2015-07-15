#include "logo.h"

#include <stdio.h>

#include "../displayhelper.h"

#include "connect.h"

namespace modes
{

Logo::Logo()
{
    DisplayHelper::showLogo();
}

int Logo::getTimeout()
{
    return 5000;
}

void Logo::listenForEvents(bool awaitMask[])
{
}

Base* Logo::doEvent(int orbId)
{
    return new ModeConnect();
}

}
