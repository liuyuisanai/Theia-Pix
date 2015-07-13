extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <display.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <systemlib/systemlib.h>

#include "datamanager.h"
#include "modes/base.h"
#include "modes/logo.h"

static bool main_thread_should_exit = false;
static bool thread_running = false;
static int deamon_task;

static int app_main_thread(int argc, char *argv[]) {
    DataManager *dm = DataManager::instance();
    modes::Base *currentMode = new modes::Logo();

    thread_running = true;

    while (!main_thread_should_exit)
    {
        modes::Base *nextMode = nullptr;
        int timeout = currentMode->getTimeout();

        dm->clearAwait();
        currentMode->listenForEvents(dm->awaitMask);

        bool hasChange = dm->wait(timeout);

        if (hasChange)
        {
            for (int i = 0; i < FD_Size; i++)
            {
                if (dm->awaitResult[i])
                {
                    nextMode = currentMode->doEvent(i);
                    if (nextMode != nullptr)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            nextMode = currentMode->doEvent(-1);
        }

        if (nextMode != nullptr)
        {
            delete currentMode;
            currentMode = nextMode;
        }

    }

    delete currentMode;
    thread_running = false;

    return 0;
}

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
