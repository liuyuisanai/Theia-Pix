#include <stdio.h>

#include "mode_class.h"
//#include "orbs_data.h"

#include <poll.h>
#include "uORB/topics/kbd_handler.h"


static bool main_thread_should_exit = false;
static bool thread_running = false;
static int deamon_task;

int app_main_thread(int argc, const char *argv[]) {

    /* temporary */
    int kbd_handler_sub = orb_subscribe(ORB_ID(kbd_handler));
    struct pollfd btnfd;
    btnfd.fd = kbd_handler_sub;
    btnfd.events = POLLIN;
    /* end of temporary */

    Mode* current_mode;
    GMainScreen main_scr("main landed screen", btnfd);
    current_mode = &main_scr;
    while (!main_thread_should_exit){
        current_mode = current_mode->execute();
    }
    //main_scr.execute();
    return 0;
}

extern "C" __EXPORT
int main(int argc, const char * const * const argv)
{
    if (argc > 1 && strcmp("start", argv[1]) == 0)
    {
        if (!thread_running)
        {
            main_thread_should_exit = false;
            deamon_task = task_spawn_cmd("leash_app",
                                         SCHED_DEFAULT,
                                         SCHED_PRIORITY_DEFAULT - 30,
                                         3000,
                                         app_main_thread,
                                         (const char **)argv);
        }
        else
        {
            printf("already running");
            /* this is not an error */
        }
    }
    else if (argc > 1 && strcmp(argv[1], "stop") == 0)
    {
        if (thread_running)
        {
            main_thread_should_exit = true;
        }
        else
        {
            printf("not started");
        }
    }
    //else if (argc > 2 && strcmp(argv[1], "show") == 0)
    //{
    //    struct leash_display_s leash_display;
    //    static orb_advert_t to_leash_display = 0;

    //    leash_display.screenId = atoi(argv[2]);
    //    if (to_leash_display > 0)
    //    {
    //        orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
    //    }
    //    else
    //    {
    //        to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
    //    }
    //}
    //else if (argc > 3 && strcmp(argv[1], "set") == 0)
    //{
    //    if (strcmp(argv[2], "battery") == 0) {
    //        struct airdog_status_s airdog_status;
    //        static orb_advert_t to_airdog_status = 0;

    //        getOrbData(ORB_ID(airdog_status), &airdog_status);

    //        printf("old airdog_battery %d\n", airdog_status.battery_remaining);

    //        airdog_status.battery_remaining = atoi(argv[3]);

    //        printf("set airdog_battery to %d\n", airdog_status.battery_remaining);

    //        if (to_airdog_status > 0)
    //        {
    //            orb_publish(ORB_ID(airdog_status), to_airdog_status, &airdog_status);
    //        }
    //        else
    //        {
    //            to_airdog_status = orb_advertise(ORB_ID(airdog_status), &airdog_status);
    //        }
    //    }
    //}
    else
    {
        printf("Wrong parameters:\n"
               "parameters:"
               "\tstart\n"
               "\tstop\n"
               );
    }
    return 0;
}
