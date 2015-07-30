#pragma once

#include "base.h"

namespace modes
{

class List : public Base
{
public:
    List();
    ~List();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);

    void setList(const char **pLines, int pCount);
    void setIndex(int x, int y);

protected:
    char **lines;
    int count;
    int xIndex;
    int yIndex;

    void show();
    void freeList();
};

}

