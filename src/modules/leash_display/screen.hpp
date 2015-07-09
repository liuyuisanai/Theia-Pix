#pragma once

class Screen
{
public:
    static void showLogo();
    static void showMain(const char *presetName, int leashBattery, int airdogBattery,
                         int airdogMode, int followMode, int landMode);
    static void showMenu(int buttons, int type, int value);
    static void showInfo(int info);
};

