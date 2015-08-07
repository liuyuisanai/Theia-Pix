#pragma once

#include "base.h"

#define ERROR_SHOW_INTERVAL 5000

namespace modes
{

class Error : public Base
{
public:
    Error();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

    void setList(const char **pLines, int pCount);
    void setIndex(int x, int y);

protected:
    bool isErrorShowed;
    int lastErrorCode;
    static int lastErrorId;
};

}

