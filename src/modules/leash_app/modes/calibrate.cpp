#include "calibrate.h"

#include <stdio.h>
#include <unistd.h>

#include <systemlib/systemlib.h>

#include <uORB/uORB.h>
#include <uORB/topics/vehicle_command.h>

#include "main.h"
#include "../datamanager.h"
#include "../displayhelper.h"
#include "../../airdog/calibrator/calibrator.hpp"
#include "../../airdog/calibrator/calibration_commons.hpp"
#include "../uorb_functions.h"

namespace modes
{

Calibrate::Calibrate(CalibrationDevice pDevice) :
    device(pDevice)
{
    switch (device)
    {
        case CalibrationDevice::LEASH_ACCEL:
            DisplayHelper::showInfo(INFO_NEXT_SIDE_UP, 0);
            calibration::calibrate_in_new_task(calibration::CALIBRATE_ACCELEROMETER);
            break;

        case CalibrationDevice::LEASH_GYRO:
            DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);
            calibration::calibrate_in_new_task(calibration::CALIBRATE_GYROSCOPE);
            break;

        case CalibrationDevice::LEASH_MAGNETOMETER:
            DisplayHelper::showInfo(INFO_CALIBRATING_HOLD_STILL, 0);
            calibration::calibrate_in_new_task(calibration::CALIBRATE_MAGNETOMETER);
            break;

        case CalibrationDevice::AIRDOG_ACCEL:
            sendAirDogCommnad(VEHICLE_CMD_PREFLIGHT_CALIBRATION, 0, 0, 0, 0, 1);
            break;

        case CalibrationDevice::AIRDOG_GYRO:
            sendAirDogCommnad(VEHICLE_CMD_PREFLIGHT_CALIBRATION, 1);
            break;

        case CalibrationDevice::AIRDOG_MAGNETOMETER:
            sendAirDogCommnad(VEHICLE_CMD_PREFLIGHT_CALIBRATION, 0, 1);
            break;

        default:
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
    awaitMask[FD_Calibrator] = 1;
}

Base* Calibrate::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_BACK))
        {
            switch (device)
            {
                case CalibrationDevice::LEASH_ACCEL:
                case CalibrationDevice::LEASH_GYRO:
                case CalibrationDevice::LEASH_MAGNETOMETER:
                    calibration::calibrate_stop();
                    break;

                case CalibrationDevice::AIRDOG_ACCEL:
                case CalibrationDevice::AIRDOG_GYRO:
                case CalibrationDevice::AIRDOG_MAGNETOMETER:
                    sendAirDogCommnad(VEHICLE_CMD_PREFLIGHT_CALIBRATION, 0, 0, 0, 0, 0, 0, 1);
                    break;
            }
            nextMode = new Main();
        }
        else if (key_pressed(BTN_OK) &&
                 CALIBRATOR_FINISH == DataManager::instance()->calibrator.status)
        {
            nextMode = new Main();
        }
    }
    else if (orbId == FD_Calibrator)
    {
        int status = DataManager::instance()->calibrator.status;
        calibration::CALIBRATION_RESULT result = DataManager::instance()->calibrator.result;

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

            case CALIBRATOR_FINISH:
                if (result == calibration::CALIBRATION_RESULT::SUCCESS)
                {
                    DisplayHelper::showInfo(INFO_SUCCESS, 0);
                }
                else
                {
                    DisplayHelper::showInfo(INFO_FAILED, 0);
                }
                break;
        }
    }

    return nextMode;
}

}
