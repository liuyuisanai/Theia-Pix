#include "calibrate.h"

#include <stdio.h>
#include <unistd.h>

#include <systemlib/systemlib.h>

#include "main.h"
#include "../datamanager.h"
#include "../displayhelper.h"
#include "../../airdog/calibrator/calibrator.hpp"
#include "../../airdog/calibrator/calibration_commons.hpp"

namespace modes
{

static int calibration_task;

static int start_calibrate_accelerometer(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    sleep(1);
    bool result = calibration::calibrate_accelerometer();

    if (result)
    {
        DisplayHelper::showInfo(INFO_SUCCESS, 0);
    }
    else
    {
        DisplayHelper::showInfo(INFO_FAILED, 0);
    }

    return 0;
}

static int start_calibrate_gyro(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    sleep(1);
    bool result = calibration::calibrate_gyroscope();

    if (result)
    {
        DisplayHelper::showInfo(INFO_SUCCESS, 0);
    }
    else
    {
        DisplayHelper::showInfo(INFO_FAILED, 0);
    }

    return 0;
}

static int start_calibrate_magnetometer(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    sleep(1);
    bool result = calibration::calibrate_magnetometer();

    if (result)
    {
        DisplayHelper::showInfo(INFO_SUCCESS, 0);
    }
    else
    {
        DisplayHelper::showInfo(INFO_FAILED, 0);
    }

    return 0;
}

Calibrate::Calibrate(CalibrationDevice pDevice) :
    device(pDevice)
{
    switch (device)
    {
        case CalibrationDevice::LEASH_ACCEL:
            calibration_task = task_spawn_cmd("leash_app",
                                              SCHED_DEFAULT,
                                              SCHED_PRIORITY_DEFAULT - 30,
                                              3000,
                                              start_calibrate_accelerometer,
                                              nullptr);
            DisplayHelper::showInfo(INFO_NEXT_SIDE_UP, 0);
            break;

        case CalibrationDevice::LEASH_GYRO:
            DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);

            calibration_task = task_spawn_cmd("leash_app",
                                              SCHED_DEFAULT,
                                              SCHED_PRIORITY_DEFAULT - 30,
                                              3000,
                                              start_calibrate_gyro,
                                              nullptr);
            break;

        case CalibrationDevice::LEASH_MAGNETOMETER:
            DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);

            calibration_task = task_spawn_cmd("leash_app",
                                              SCHED_DEFAULT,
                                              SCHED_PRIORITY_DEFAULT - 30,
                                              3000,
                                              start_calibrate_magnetometer,
                                              nullptr);
            break;
    }
}

int Calibrate::getTimeout()
{
    return -1;
}

void Calibrate::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
    if (device == CalibrationDevice::LEASH_ACCEL ||
            device == CalibrationDevice::LEASH_MAGNETOMETER)
    {
        awaitMask[FD_Calibrator] = 1;
    }
}

Base* Calibrate::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_BACK))
        {
            calibration::calibrate_stop();
            nextMode = new Main();
        }
        else if (key_pressed(BTN_OK) && calibration::calibrate_finished())
        {
            nextMode = new Main();
        }
    }
    else if (orbId == FD_Calibrator)
    {
        int status = DataManager::instance()->calibrator.status;

        switch (status)
        {
            case CALIBRATOR_DETECTING_SIDE:
                DisplayHelper::showInfo(INFO_NEXT_SIDE_UP, 0);
                break;

            case CALIBRATOR_CALIBRATING:
                DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);
                break;

            case CALIBRATOR_DANCE:
                DisplayHelper::showInfo(INFO_CALIBRATING_DANCE, 0);
                break;
        }
    }

    return nextMode;
}

}
