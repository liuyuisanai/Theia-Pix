#include "displayhelper.h"

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
