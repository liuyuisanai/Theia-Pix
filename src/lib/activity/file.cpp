#include "file.hpp"

#include <cstdio>

namespace AirDog
{
namespace activity
{
namespace file
{

constexpr char root[] = "/fs/microsd";

bool
get_path(uint8_t activity, uint8_t attribute, char (&name)[PATH_MAX])
{
	switch (attribute)
	{
	case 0:
		snprintf(name, sizeof name, "%s/activity/%03u.bin",
				root, unsigned(activity));
		break;
	default:
		*name = '\0';
		break;
	}
}

bool
has_valid_content(uint8_t activity, uint8_t attribute, const char pathname[])
{
	// TODO Add verification here.
	return true;
}

bool
update_activity(uint8_t activity, uint8_t attribute)
{
	// TODO Apply parameters if activity is current one.
	return true;
}

}
// end of namespace files
}
// end of namespace activity
}
// end of namespace AirDog
