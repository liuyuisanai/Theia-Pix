#pragma once

#include "datamanager.h"

#define BTN_POWER  0x001
#define BTN_PLAY   0x004
#define BTN_UP     0x008
#define BTN_DOWN   0x040
#define BTN_LEFT   0x020
#define BTN_RIGHT  0x100
#define BTN_TO_ME  0x200
#define BTN_TO_H   0x080
#define BTN_MODE   0x010
#define BTN_FUTURE 0x002
#define BTN_OK     BTN_POWER
#define BTN_BACK   BTN_MODE

bool key_pressed(int key_mask);
bool key_ShortPressed(int key_mask);
bool key_LongPressed(int key_mask);
