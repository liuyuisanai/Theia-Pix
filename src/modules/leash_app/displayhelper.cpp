#include "displayhelper.h"

#include <stdio.h>
#include <string.h>

#include <uORB/uORB.h>
#include <uORB/topics/leash_display.h>

static orb_advert_t to_leash_display = 0;

void DisplayHelper::showLogo()
{
    struct leash_display_s leash_display;

    leash_display.screenId = LEASHDISPLAY_LOGO;
    if (to_leash_display > 0)
    {
        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    }
    else
    {
        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    }
}

void DisplayHelper::showMain(int mode, const char *presetName, int airdogMode, int followMode, int landMode)
{
    struct leash_display_s leash_display;

    leash_display.screenId = LEASHDISPLAY_MAIN;
    leash_display.airdogMode = airdogMode;
    leash_display.followMode = followMode;
    leash_display.landMode = landMode;
    leash_display.mainMode = mode;

    if (presetName != nullptr)
    {
        strncpy(leash_display.presetName, presetName, 20);
    }
    else
    {
        leash_display.presetName[0] = 0;
    }

    if (to_leash_display > 0)
    {
        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    }
    else
    {
        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    }
}

void DisplayHelper::showMenu(int buttons, int type, int value, const char *presetName)
{
    struct leash_display_s leash_display;

    leash_display.screenId = LEASHDISPLAY_MENU;
    leash_display.menuButtons = buttons;
    leash_display.menuType = type;
    leash_display.menuValue = value;
    if (presetName != nullptr)
    {
        strncpy(leash_display.presetName, presetName, 20);
    }
    else
    {
        leash_display.presetName[0] = 0;
    }

    if (to_leash_display > 0)
    {
        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    }
    else
    {
        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    }
}

void DisplayHelper::showInfo(int info, int error)
{
    struct leash_display_s leash_display;

    leash_display.screenId = LEASHDISPLAY_INFO;
    leash_display.infoId = info;
    leash_display.infoError = error;

    if (to_leash_display > 0)
    {
        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    }
    else
    {
        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    }
}

void DisplayHelper::showList(const char **lines, int lineCount, int x, int y)
{
    struct leash_display_s leash_display;

    leash_display.screenId = LEASHDISPLAY_LIST;
    leash_display.lineCount = lineCount - y;

    if (leash_display.lineCount > LEASHDISPLAY_LINE_COUNT)
    {
        leash_display.lineCount = LEASHDISPLAY_LINE_COUNT;
    }

    for (int i = 0; i < LEASHDISPLAY_LINE_COUNT; i++)
    {
        int j = y + i;

        if (j < lineCount)
        {
            int length = strlen(lines[j]);

            if (x < length)
            {
                strncpy(&leash_display.lines[i][0], lines[j] + x, LEASHDISPLAY_LINE_LENGH);
                leash_display.lines[i][LEASHDISPLAY_LINE_LENGH - 1] = 0;
            }
            else
            {
                leash_display.lines[i][0] = 0;
            }
        }
    }

    if (to_leash_display > 0)
    {
        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    }
    else
    {
        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    }
}
