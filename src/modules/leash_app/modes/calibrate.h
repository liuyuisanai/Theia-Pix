#pragma once

#include "base.h"

namespace modes
{

enum class CalibrationDevice
{
    LEASH_ACCEL,
    LEASH_GYRO,
    LEASH_MAGNETOMETER,
    AIRDOG_ACCEL,
    AIRDOG_GYRO,
    AIRDOG_MAGNETOMETER,
};

class Calibrate : public Base
{
public:
    Calibrate(CalibrationDevice pDevice);

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

private:
    CalibrationDevice device;
};

}

