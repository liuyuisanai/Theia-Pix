#pragma once

#include "list.h"

namespace modes
{
class Service : public List
{
public:
    Service();
    ~Service();

    virtual int getTimeout();
    virtual void listenForEvents(bool awaitMask[]);
    virtual Base* doEvent(int orbId);
};

} // end of namespace modes
