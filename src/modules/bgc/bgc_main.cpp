#include <nuttx/config.h>
#include "bgc.hpp"

extern "C" __EXPORT int bgc_main(int argc, char *argv[]);

int bgc_main(int argc, char *argv[]) {
    printf("[bgc] bgc_main - started\n");
    sleep(3);        // Give BGC some time to initialize and turn motors on.
    for ( ; ; ) {
        BGC::BGC bgc;
        bgc.Run();   // We will only return from Run if a fatal error occurs. And instead of
        sleep(10);   // dying, we'll give some time for things to settle down, and try again.
    }
    return 0;
}
