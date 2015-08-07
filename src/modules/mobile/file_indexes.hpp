#pragma once

#include <limits.h>

#include <activity/file.hpp>

#include "protocol.h"


/*
 * Index structure.
 */

enum FileCatalog : uint8_t
{
	INVALID,
	ACTIVITY,
	AUTHENCITY,
	LOGS,
	PUBLIC,
};

FileCatalog
catalog(file_index_t index)
{
	if (index == 0) { return FileCatalog::AUTHENCITY; }
	if (index == 1) { return FileCatalog::PUBLIC; }
	if ((index >> 16) == 0x0001) { return FileCatalog::ACTIVITY; }
	if ((index >> 24) == 0x80) { return FileCatalog::LOGS; }
	return FileCatalog::INVALID;
}

using filename_buf_t = char[PATH_MAX];

/*
 * Activity files.
 */

void
parse_activity_index(file_index_t index, uint8_t & activity, uint8_t & attribute)
{
	activity = (index >> 8) & 0xFF;
	attribute = index & 0xFF;
}

bool
get_activity_filename(file_index_t index, filename_buf_t & pathname)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return activity::file::get_path(activity, attribute, pathname);
}

bool
is_activity_index_valid(file_index_t index)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return activity::file::has_valid_id(activity, attribute);
}

bool
is_activity_file_valid(const char tmp_path[], file_index_t index)
{
	using namespace AirDog;
	uint8_t activity, attribute;
	parse_activity_index(index, activity, attribute);
	return activity::file::has_valid_content(activity, attribute, tmp_path);
}


/*
 * General files.
 */

bool
get_filename(file_index_t index, filename_buf_t & name)
{
	FileCatalog c = catalog(index);
	bool ok = false;
	switch (c)
	{
	case FileCatalog::ACTIVITY:
		ok = get_activity_filename(index, name);
		break;
	case FileCatalog::PUBLIC:
		strncpy(name, "/fs/microsd/mobile/public.dat", sizeof name);
		ok = true;
		break;
	default:
		dbg("No name for file index 0x%08x.\n", index);
		*name = '\0';
		break;
	}
	return ok;
}

bool
is_file_index_valid(file_index_t index)
{
	FileCatalog c = catalog(index);
	return ((c == FileCatalog::ACTIVITY) and is_activity_index_valid(index))
		or (c != FileCatalog::INVALID);
}

bool
is_file_writable(file_index_t index)
{
	FileCatalog c = catalog(index);
	return c == FileCatalog::ACTIVITY or c == FileCatalog::PUBLIC;
}

bool
is_file_content_valid(const char tmp_path[], file_index_t index)
{
	FileCatalog c = catalog(index);
	switch (c)
	{
	case FileCatalog::ACTIVITY:
		return is_activity_file_valid(tmp_path, index);
	case FileCatalog::PUBLIC:
		return true;
	default:
		return false;
	}
}
