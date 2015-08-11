#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum commander_error_code
{
    COMMANDER_ERROR_OK = 0,
    COMMANDER_ERROR_NOT_ACTIVATED = 1,
};

__EXPORT int commander_set_error(int error_code);

#ifdef __cplusplus
}
#endif
