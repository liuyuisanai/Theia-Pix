#include <stdio.h>
#include "mode_class.h"

int main() {
    Mode* current_mode;
    GMainScreen main_scr("main landed screen");
    current_mode = &main_scr;
    while (current_mode) {
        current_mode = current_mode->execute();
    }
    //main_scr.execute();
    return 0;
}

class leash_app{
    public:
        ~leash_app() {};
        void start();
        void info();
    private:
        ModeLandScreen land_menu;
};
