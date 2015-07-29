#pragma once

#include <uORB/topics/leash_display.h>

class Screen
{
public:
    static void init();
    static void showLogo();
    static void showMain(int mode, const char *presetName, int leashBattery, int airdogBattery,
                         int airdogMode, int followMode, int landMode);
    static void showMenu(int buttons, int type, int value, const char *presetName);
    static void showInfo(int info, int error);
    static void showList(LeashDisplay_Lines lines, int lineCount);
};

