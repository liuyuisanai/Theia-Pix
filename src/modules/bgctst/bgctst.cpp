#include <nuttx/config.h>
#include "bgc.hpp"

extern "C" __EXPORT int bgctst_main(int argc, char *argv[]);

int bgctst_main(int argc, char *argv[]) {
    printf("bgctst_main\n");
    for ( ; ; ) {
        BGC::BGC bgc;
        bgc.Run();   // We will only return from Run if a fatal error occurs. And instead of
        sleep(10);   // dying, we'll give some time for things to settle down, and try again.
    }
    return 0;
}
