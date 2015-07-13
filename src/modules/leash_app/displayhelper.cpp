#include "displayhelper.h"

#include <string.h>

#include <uORB/uORB.h>
#include <uORB/topics/leash_display.h>

void DisplayHelper::showLogo()
{
    struct leash_display_s leash_display;
    static orb_advert_t to_leash_display = 0;

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

void DisplayHelper::showMain(int mode, const char *presetName, int leashBattery,
                             int airdogBattery, int airdogMode, int followMode, int landMode)
{

}

void DisplayHelper::showMenu(int buttons, int type, int value, const char *presetName)
{
    struct leash_display_s leash_display;
    static orb_advert_t to_leash_display = 0;

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

}
