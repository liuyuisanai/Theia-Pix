#include "connect.h"
#include "menu.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "../datamanager.h"
#include "../button_handler.h"
#include "../displayhelper.h"


#define _BLUETOOTH21_BASE       0x2d00

#define PAIRING_ON          _IOC(_BLUETOOTH21_BASE, 0)
#define PAIRING_OFF         _IOC(_BLUETOOTH21_BASE, 1)


namespace modes
{

ModeConnect::ModeConnect()
    : CurrentState(ModeState::UNKNOWN)
{
    doEvent(0);
}

ModeConnect::~ModeConnect() { }

void ModeConnect::listenForEvents(bool awaitMask[])
{
    awaitMask[FD_KbdHandler] = 1;
    awaitMask[FD_BLRHandler] = 1;
}

int ModeConnect::getTimeout()
{
    return -1;
}

Base* ModeConnect::doEvent(int orbId)
{
    Base *nextMode = nullptr;

    if (orbId == FD_BLRHandler)
    {
        getConState();
        DOG_PRINT("[modes]{connection} doingEvent state: %d\n", CurrentState);
        if (CurrentState == ModeState::CONNECTING) 
        {
            DisplayHelper::showInfo(INFO_CONNECTING_TO_AIRDOG, 0);
        }
        else if (CurrentState == ModeState::CONNECTED) 
        {
            nextMode = new Menu();
        }
        else if (CurrentState == ModeState::DISCONNECTED) 
        {
            DisplayHelper::showInfo(INFO_CONNECTION_LOST, 0);
        }
        else if (CurrentState == ModeState::NOT_PAIRED) 
        {
            DisplayHelper::showInfo(INFO_PAIRING, 0);
        }
        else if (CurrentState == ModeState::PAIRING) 
        {
            DisplayHelper::showInfo(INFO_PAIRING, 0);
        }
        else {
            DisplayHelper::showInfo(INFO_FAILED, 0);
        }
    }
    else if (orbId == FD_KbdHandler)
    {
        if (key_pressed(BTN_OK)) 
        {
            if (CurrentState == ModeState::NOT_PAIRED)
            {
                DOG_PRINT("[modes]{connection} start pairing!\n");
                BTPairing(true);
            }
        }
        else if (key_LongPressed(BTN_MODE)) 
        {
            if (CurrentState == ModeState::PAIRING)
            {
                DOG_PRINT("[modes]{connection} stop pairing!\n");
                BTPairing(false);
            }
        }
        else if (key_LongPressed(BTN_MODE))
        {
            if (CurrentState == ModeState::UNKNOWN)
                nextMode = new Menu();
        }
    }
    return nextMode;
}

void ModeConnect::getConState()
{
    DataManager *dm = DataManager::instance();
    DOG_PRINT("[modes]{connetion}bt_report: %d\n", dm->bt_handler.global_state);
    switch(dm->bt_handler.global_state) {
        case NO_PAIRED_DEVICES:
            CurrentState = ModeState::NOT_PAIRED;
            break;
        case PAIRING:
            CurrentState = ModeState::PAIRING;
            break;
        case CONNECTING:
            CurrentState = ModeState::CONNECTING;
            break;
        case CONNECTED:
            CurrentState = ModeState::CONNECTED;
            break;
        default:
            CurrentState = ModeState::UNKNOWN;
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
