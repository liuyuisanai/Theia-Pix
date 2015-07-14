#include "button_handler.h"
bool
key_pressed(int key_mask) {
    DataManager *dm = DataManager::instance();
    if (dm->kbd_handler.buttons == key_mask) {
        DOG_PRINT("[button_handler] Key %d event %d\n", dm->kbd_handler.buttons, dm->kbd_handler.event);
        fflush(stderr);
        return true;
    }
    return false;
};

bool
key_ShortPressed(int key_mask) {
    DataManager *dm = DataManager::instance();
    if (dm->kbd_handler.buttons == key_mask) {
        DOG_PRINT("[button_handler] Key %d event %d\n", dm->kbd_handler.buttons, dm->kbd_handler.event);
        fflush(stderr);
        if (dm->kbd_handler.event == 0)
            return true;
    }
    return false;
};

bool
key_LongPressed(int key_mask) {
    DataManager *dm = DataManager::instance();
    if (dm->kbd_handler.buttons == key_mask) {
        DOG_PRINT("[button_handler] Key %d event %d\n", dm->kbd_handler.buttons, dm->kbd_handler.event);
        fflush(stderr);
        if (dm->kbd_handler.event == 1)
            return true;
    }
    return false;
};
