#pragma once

#include <limits.h>

#include <cstdint>

namespace AirDog
{
namespace activity
{
namespace file
{

/*
 * Inlines.
 */

static inline bool
has_valid_id(uint8_t activity, uint8_t attribute)
{ return (activity < 10) and (attribute == 0); }


/*
 * Exported functions.
 */

__EXPORT bool
get_path(uint8_t activity, uint8_t attribute, char (&pathname)[PATH_MAX]);

__EXPORT bool
has_valid_content(uint8_t activity, uint8_t attribute, const char pathname[]);

__EXPORT bool
update_activity(uint8_t activity, uint8_t attribute);

}
// end of namespace files
}
// end of namespace activity
}
// end of namespace AirDog
