#include "connect.h"
#include "menu.h"
#include "acquiring_gps.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "../datamanager.h"
#include "../button_handler.h"
#include "../displayhelper.h"

#include "../../mavlink/mavlink_defines.h"

#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)


namespace modes
{

ModeConnect::ModeConnect(State Current) : 
    forcing_pairing(false),
    currentState(Current)
{
    if (Current == State::PAIRING)
    {
        forcing_pairing = true;
        BTPairing();
        DisplayHelper::showInfo(INFO_PAIRING);
    }
    else
    {
        doEvent(-1);
    }
}

ModeConnect::~ModeConnect() { }

void ModeConnect::listenForEvents(bool awaitMask[])
{
    switch (currentState)
    {
        case State::UNKNOWN:
        case State::NOT_PAIRED:
        case State::PAIRING:
        case State::DISCONNECTED:
        case State::CONNECTING:
        case State::CONNECTED:
            awaitMask[FD_BLRHandler] = 1;
            break;

        case State::CHECK_MAVLINK:
            awaitMask[FD_MavlinkStatus] = 1;
            break;
    }

    awaitMask[FD_KbdHandler] = 1;
}

int ModeConnect::getTimeout()
{
    return -1;
}

Base* ModeConnect::doEvent(int orbId)
{
    Base *nextMode = nullptr;
    if (forcing_pairing)
    {
        if (currentState == State::PAIRING)
            forcing_pairing = false;
    }
    else
        getConState();

    if (currentState == State::CONNECTING)
    {
        DisplayHelper::showInfo(INFO_CONNECTING_TO_AIRDOG);
    }
    else if (currentState == State::CONNECTED)
    {
        currentState = State::CHECK_MAVLINK;
        time(&startTime);
    }
    else if (currentState == State::DISCONNECTED)
    {
        DisplayHelper::showInfo(INFO_CONNECTION_LOST);
    }
    else if (currentState == State::NOT_PAIRED)
    {
        DisplayHelper::showInfo(INFO_NOT_PAIRED);
    }
    else if (currentState == State::PAIRING)
    {
        DisplayHelper::showInfo(INFO_PAIRING);
    }
    else {
        DisplayHelper::showInfo(INFO_FAILED);
    }

    if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_OK)) 
        {
            if (currentState == State::NOT_PAIRED)
            {
                DOG_PRINT("[modes]{connection} start pairing!\n");
                DisplayHelper::showInfo(INFO_NOT_PAIRED);
                BTPairing(true);
            }
        }
        else if (key_pressed(BTN_MODE)) 
        {
            switch (currentState)
            {
                case State::NOT_PAIRED:
                case State::CONNECTING:
                case State::UNKNOWN:
                    nextMode = new Menu();
                    break;
                case State::PAIRING:
                    DOG_PRINT("[modes]{connection} stop pairing!\n");
                    BTPairing(false);
                    break;

            }
        }
    }
    else if (orbId == FD_MavlinkStatus && currentState == State::CHECK_MAVLINK)
    {
        int v = DataManager::instance()->mavlink_received_stats.version;

        if (v == 0)
        {
            // mavlink version not received yet
            time_t now;
            time(&now);

            if ((int)now -(int)startTime > MAVLINK_CHECK_INTERVAL)
            {
                DisplayHelper::showInfo(INFO_COMMUNICATION_FAILED);
            }
        }
        else if (v != MAVLINK_VERSION)
        {
            // invalid mavlink version
            DisplayHelper::showInfo(INFO_COMMUNICATION_FAILED);
        }
        else
        {
            // invalid mavlink version
            nextMode = new Acquiring_gps();
        }
    }

    // Check if we are in service screen
    Base* service = checkServiceScreen(orbId);
    if (service)
        nextMode = service;

    return nextMode;
}

void ModeConnect::getConState()
{
    DataManager *dm = DataManager::instance();
    switch(dm->bt_handler.global_state) {
        case INITIALIZING :
        case CONNECTING:
            if (currentState == State::DISCONNECTED)
            {
                break;
            }
            else 
            {
                currentState = State::CONNECTING;
                break;
            }
        case NO_PAIRED_DEVICES:
            currentState = State::NOT_PAIRED;
            break;
        case PAIRING:
            currentState = State::PAIRING;
            break;
        case CONNECTED:
            if (currentState != State::CHECK_MAVLINK)
            {
                currentState = State::CONNECTED;
            }
            break;
        default:
            currentState = State::UNKNOWN;
            break;
    }
}

void ModeConnect::BTPairing(bool start)
{
    int fd = open("/dev/btctl", 0);

    if (fd > 0) {
        if (start)
            ioctl(fd, PAIRING_ON, 0);
        else
            ioctl(fd, PAIRING_OFF, 0);
    }

    close(fd);
}

} //end of namespace modes
