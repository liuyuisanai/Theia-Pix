#include "mode_class.h"

Mode* 
ModeLandScreen::execute() {
    printf("Mode name \"%s\"\n", name.c_str());
    return 0;
}

Mode* 
ModeFlyScreen::execute() {
    printf("Mode name \"%s\"\n", name.c_str());
    return 0;
}

/* === separate screens classes === */
/*          WHILE LANDED            */

Mode*
Startup::execute(){
//if there is no pairing to drone, do it
if (not_paired) {
    Mode* new_mode = new Pairing("Pairing process was initialized automatically or by user");
    return new_mode;
}
if (not_connected) {
    Mode* new_mode = new NoConnect("Connection to drone was not established during this session", /*was connected?*/ false);
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
    printf("Changing to no connection\n");
    Mode* new_mode = new NoConnect("No connection with drone during landed phase");
    return new_mode;
}

Mode* 
NoConnect::execute(){
    ModeLandScreen::execute();
    printf("No connection");
    return 0;
}
