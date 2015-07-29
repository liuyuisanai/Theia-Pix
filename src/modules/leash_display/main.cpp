extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <display.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "images/images.h"
#include "datamanager.hpp"
#include "screen.hpp"

#include <systemlib/systemlib.h>

#include <uORB/uORB.h>
#include <uORB/topics/airdog_status.h>
#include <uORB/topics/leash_display.h>

static bool main_thread_should_exit = false;
static bool thread_running = false;
static int deamon_task;


static int leash_convert_voltage_to_percent(float v)
{
    static float lastV = 0;
    struct {
        float v;
        int t;
    } data[] = {
    {4.2,  0},
    {4.1,  20},
    {4.0,  40},
    {3.94, 60},
    {3.87, 80},
    {3.82, 100},
    {3.79, 120},
    {3.77, 140},
    {3.60, 160},
    {3.5, 180},
    {0, -1},
    };
    const double maxTime = 180;
    double t = 0;
    int p = 0;

    // ignore small changes in voltage
    // to avoid shaking of battery level
    if (fabsf(lastV -v) < 0.05f) {
        v = lastV;
    }
    else
    {
        lastV = v;
    }

    for (int i = 0; data[i].t != -1; i++)
    {
        if (data[i].v > v) {
            t = data[i].t;
        }
        else
        {
            break;
        }
    }

    if (t > maxTime)
    {
        p = 0;
    }
    else
    {
        p = (int) ((maxTime - t)/maxTime * 100.0);
    }

    return p;
}

static int leash_display_thread_main(int argc, char *argv[])
{
    DataManager dm;

    dm.clearAwaitMask();
    dm.awaitMask[FD_LeashDisplay] = true;

    thread_running = true;
    int currentScreenId = 0;
    int lb = 100;

    printf("leash_display started\n");

    Screen::init();

    while (!main_thread_should_exit)
    {
        bool hasChanges = dm.wait(-1);

        if (dm.leash_display.screenId != currentScreenId)
        {
            display_clear();

            switch (dm.leash_display.screenId)
            {
                case LEASHDISPLAY_NONE:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;
                    break;

                case LEASHDISPLAY_LOGO:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;
                    Screen::showLogo();
                    break;

                case LEASHDISPLAY_MAIN:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;
                    dm.awaitMask[FD_AirdogStatus] = true;
                    dm.awaitMask[FD_SystemPower] = true;

                    lb = leash_convert_voltage_to_percent(dm.system_power.voltage5V_v);
                    Screen::showMain(dm.leash_display.mainMode, dm.leash_display.presetName, lb, dm.airdog_status.battery_remaining,
                                     dm.leash_display.airdogMode, dm.leash_display.followMode, dm.leash_display.landMode);
                    break;

                case LEASHDISPLAY_MENU:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;

                    Screen::showMenu(dm.leash_display.menuButtons, dm.leash_display.menuType,
                                     dm.leash_display.menuValue, dm.leash_display.presetName);
                    break;

                case LEASHDISPLAY_INFO:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;

                    Screen::showInfo(dm.leash_display.infoId, dm.leash_display.infoError);
                    break;

                case LEASHDISPLAY_LIST:
                    dm.clearAwaitMask();
                    dm.awaitMask[FD_LeashDisplay] = true;

                    Screen::showList(dm.leash_display.lines, dm.leash_display.lineCount);
                    break;
            }

            display_redraw_all();

            currentScreenId = dm.leash_display.screenId;
        }
        else if (hasChanges)
        {
            display_clear();

            switch (dm.leash_display.screenId)
            {
                case LEASHDISPLAY_NONE:
                    break;

                case LEASHDISPLAY_LOGO:
                    Screen::showLogo();
                    break;

                case LEASHDISPLAY_MAIN:
                    lb = leash_convert_voltage_to_percent(dm.system_power.voltage5V_v);
                    Screen::showMain(dm.leash_display.mainMode, dm.leash_display.presetName, lb, dm.airdog_status.battery_remaining,
                                     dm.leash_display.airdogMode, dm.leash_display.followMode, dm.leash_display.landMode);
                    break;

                case LEASHDISPLAY_MENU:
                    Screen::showMenu(dm.leash_display.menuButtons, dm.leash_display.menuType,
                                     dm.leash_display.menuValue, dm.leash_display.presetName);
                    break;

                case LEASHDISPLAY_INFO:
                    Screen::showInfo(dm.leash_display.infoId, dm.leash_display.infoError);
                    break;

                case LEASHDISPLAY_LIST:
                    Screen::showList(dm.leash_display.lines, dm.leash_display.lineCount);
                    break;

            }
            display_redraw_all();
        }

        if (hasChanges)
        {
            /*
            aMode = aMode == AIRDOGMODE_PAUSE ? AIRDOGMODE_NONE : aMode + 1;
            fMode = fMode == FOLLOW_PATH ? FOLLOW_ABS : FOLLOW_PATH;
            lMode = lMode == LAND_HOME ? LAND_SPOT : LAND_HOME;
            lb--;
            if (lb < 0)
            {
                lb = 100;
            }

            menuButtons = menuButtons >= 3 ? 0 : menuButtons + 1;
            menuType = ++menuType == MENUTYPE_MAX ? 0 : menuType;
            menuValue = ++menuValue == 1 ? 0 : menuValue;
            info = ++info == INFO_MAX ? 0 : info;

            mainMode = ++mainMode == MAINSCREEN_MAX ? 0 : mainMode;

            menuType = menuType < MENUTYPE_SELECT ? MENUTYPE_SELECT: menuType;
            printf("mode %d buttons %x\n", status.mode, status.buttons);
            printf("screenId %d\n", status.screenId);
            printf("airdog_battery %d\n", status.airdog_battery);
            printf("leash_battery %.5f\n", (double)status.leash_battery);
            */
        }
    }

    thread_running = false;

    printf("leash_display ended");

    return 0;
}


static void getOrbData(const struct orb_metadata *meta, void *data) {
    int fd = orb_subscribe(meta);
    orb_copy(meta, fd, data);
    orb_unsubscribe(fd);
}

int
main(int argc, const char * const * const argv)
{
    if (argc > 1 && strcmp("start", argv[1]) == 0)
    {
        if (!thread_running)
        {
            main_thread_should_exit = false;
            deamon_task = task_spawn_cmd("leash_display",
                                         SCHED_DEFAULT,
                                         SCHED_PRIORITY_DEFAULT - 30,
                                         3000,
                                         leash_display_thread_main,
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
    else if (argc > 2 && strcmp(argv[1], "show") == 0)
    {
        struct leash_display_s leash_display;
        static orb_advert_t to_leash_display = 0;

        leash_display.screenId = atoi(argv[2]);
        if (to_leash_display > 0)
        {
            orb_publish(ORB_ID(leash_display), to_leash_display, &leash_display);
        }
        else
        {
            to_leash_display = orb_advertise(ORB_ID(leash_display), &leash_display);
        }
    }
    else if (argc > 3 && strcmp(argv[1], "set") == 0)
    {
        if (strcmp(argv[2], "battery") == 0) {
            struct airdog_status_s airdog_status;
            static orb_advert_t to_airdog_status = 0;

            getOrbData(ORB_ID(airdog_status), &airdog_status);

            printf("old airdog_battery %d\n", airdog_status.battery_remaining);

            airdog_status.battery_remaining = atoi(argv[3]);

            printf("set airdog_battery to %d\n", airdog_status.battery_remaining);

            if (to_airdog_status > 0)
            {
                orb_publish(ORB_ID(airdog_status), to_airdog_status, &airdog_status);
            }
            else
            {
                to_airdog_status = orb_advertise(ORB_ID(airdog_status), &airdog_status);
            }
        }
    }
    else
    {
        printf("Wrong parameters:\n"
               "parameters:"
               "\tstart\n"
               "\tstop\n"
               "\tshow screenId\n"
               );
    }

    if (0){
        printf("command show:\n");

        display_clear();

        if (strcmp("logo", argv[2]) == 0)
        {
            printf("show logo:\n");
            display_bitmap(0, 0, imageInfo[0].w, imageInfo[0].h,
                    imageData + imageInfo[0].offset);
        }

        display_redraw_all();
    }

    return 0;
}
