#include "connect.h"
#include "menu.h"
#include "acquiring_gps.h"

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

ModeConnect::ModeConnect(ModeState Current)
    : currentState(Current)
{
    if (Current == ModeState::PAIRING)
    {
        BTPairing();
    }
    else
    {
        doEvent(-1);
    }
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
    getConState();
    if (orbId == FD_BLRHandler)
    {
        if (currentState == ModeState::CONNECTING) 
        {
            DisplayHelper::showInfo(INFO_CONNECTING_TO_AIRDOG, 0);
        }
        else if (currentState == ModeState::CONNECTED) 
        {
            nextMode = new Acquiring_gps();
        }
        else if (currentState == ModeState::DISCONNECTED) 
        {
            DisplayHelper::showInfo(INFO_CONNECTION_LOST, 0);
        }
        else if (currentState == ModeState::NOT_PAIRED) 
        {
            DisplayHelper::showInfo(INFO_NOT_PAIRED, 0);
        }
        else if (currentState == ModeState::PAIRING) 
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
            if (currentState == ModeState::NOT_PAIRED)
            {
                DOG_PRINT("[modes]{connection} start pairing!\n");
                DisplayHelper::showInfo(INFO_NOT_PAIRED, 0);
                BTPairing(true);
            }
        }
        else if (key_ShortPressed(BTN_MODE))
        {
            if (currentState == ModeState::NOT_PAIRED)
            {
                nextMode = new Menu();
            }
        }
        else if (key_LongPressed(BTN_MODE)) 
        {
            if (currentState == ModeState::PAIRING)
            {
                DOG_PRINT("[modes]{connection} stop pairing!\n");
                BTPairing(false);
            }
            else if (currentState == ModeState::UNKNOWN)
            {
                DOG_PRINT("[modes]{connection} unknown connection state!\n");
                nextMode = new Menu();
            }
            else if (currentState == ModeState::CONNECTING)
            {
                DOG_PRINT("[modes]{connection} connecting now, switching to main menu!\n");
                nextMode = new Menu();
            }
        }
    }
    return nextMode;
}

void ModeConnect::getConState()
{
    DataManager *dm = DataManager::instance();
    switch(dm->bt_handler.global_state) {
        case INITIALIZING :
        case CONNECTING:
            if (currentState == ModeState::DISCONNECTED)
            {
                break;
            }
            else 
            {
                currentState = ModeState::CONNECTING;
                break;
            }
        case NO_PAIRED_DEVICES:
            currentState = ModeState::NOT_PAIRED;
            break;
        case PAIRING:
            currentState = ModeState::PAIRING;
            break;
        case CONNECTED:
            currentState = ModeState::CONNECTED;
            break;
        default:
            currentState = ModeState::UNKNOWN;
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
