#include <nuttx/config.h>
#include <string.h>
#include "bgc.hpp"

extern "C" __EXPORT int bgc_main(int argc, char *argv[]);

int bgc_main(int argc, char *argv[]) {
    if ( argc == 2 && !strcmp(argv[1], "start") ) {
        if ( !BGC::BGC::Start_thread() ) {
            return 2;
        }
    } else if ( argc == 2 && !strcmp(argv[1], "stop") ) {
        if ( !BGC::BGC::Stop_thread() ) {
            return 3;
        }
    } else {
        printf("usage: bgc [start|stop]\n");
        return 1;
    }
    
    return 0;
}
