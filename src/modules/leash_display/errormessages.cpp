#include "errormessages.hpp"

const char *getErrorMessageText(int errorCode)
{
    const char *result = nullptr;

    switch (errorCode)
    {
        case COMMANDER_ERROR_OK:
            result = "OK";
            break;
        case 1:
            result = "Please activate\nyour Airdog\nvia mobile app";
            break;
    }

    return result;
}
