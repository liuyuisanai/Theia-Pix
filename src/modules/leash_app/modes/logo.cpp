#include "logo.h"

#include <stdio.h>

#include "../displayhelper.h"

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
    printf("Hu hu hu jeee ! \n");
    return nullptr;
}

}
