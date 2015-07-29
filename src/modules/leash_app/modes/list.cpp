#include "list.h"

#include <stdio.h>
#include <string.h>

#include "../displayhelper.h"

#include "connect.h"

namespace modes
{

List::List()
{
    const char *m[] = {
        "Test data",
        "to show",
        "How list mode",
        "works",
    };

    lines = nullptr;
    count = 0;

    setList(m, 4);
}

List::~List()
{

    for (int i = 0; i < count; i++)
    {
        delete []lines[i];
    }

    delete[] lines;
}

void List::setList(const char **pLines, int pCount)
{
    count = pCount;

    lines = new char*[count];

    for (int i = 0; i < count; i++)
    {
        int len = strlen(pLines[i]);
        lines[i] = new char[len + 1];
        strcpy(lines[i], pLines[i]);
    }

    xIndex = 0;
    yIndex = 0;

    show();
}

void List::show()
{
    DisplayHelper::showList((const char**)lines, count, xIndex, yIndex);
}

int List::getTimeout()
{
    return -1;
}

void List::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
}

Base* List::doEvent(int orbId)
{
    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_DOWN))
        {
            if (yIndex < count - LEASHDISPLAY_LINE_COUNT)
            {
                yIndex++;
                show();
            }
        }
        else if (key_pressed(BTN_UP))
        {
            if (yIndex > 0)
            {
                yIndex--;
                show();
            }
        }
        else if (key_pressed(BTN_LEFT))
        {
            if (xIndex > 0)
            {
                xIndex--;
                show();
            }
        }
        else if (key_pressed(BTN_RIGHT))
        {
            // get max width
            int maxLength = 0;
            for (int i = 0; i < LEASHDISPLAY_LINE_COUNT; i++)
            {
                int j = yIndex + i;
                if (j < count)
                {
                    int length = strlen(lines[j]);
                    if (length > maxLength)
                    {
                        maxLength = length;
                    }
                }
            }

            if (xIndex < maxLength)
            {
                xIndex++;
                show();
            }
        }
    }

    return nullptr;
}

}
