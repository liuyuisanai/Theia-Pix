#include "mode_class.h"

Mode* 
ModeLandScreen::execute() {
    DOG_PRINT("Mode name \"%s\"\n", name);
    return 0;
}

Mode* 
ModeFlyScreen::execute() {
    DOG_PRINT("Mode name \"%s\"\n", name);
    return 0;
}

/* === separate screens classes === */
/*          WHILE LANDED            */

Mode*
Startup::execute(){
//if there is no pairing to drone, do it
if (not_paired) {
    Mode* new_mode = new Pairing("Pairing process was initialized automatically or by user", btnfd);
    return new_mode;
}
if (not_connected) {
    bool was_connected = false;
    Mode* new_mode = new NoConnect("Connection to drone was not established during this session", btnfd, was_connected);
    return new_mode;
}
}

Mode*
Pairing::execute(){
    return 0;
}

Mode* 
GMainScreen::execute(){
    ModeLandScreen::execute();
    return 0;
}

Mode* 
NoConnect::execute(){
    //ModeLandScreen::execute();
    DOG_PRINT("No connection");
    return 0;
}
