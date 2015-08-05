#pragma once

#include "protocol.h"

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

void
parse_activity_index(file_index_t index, uint8_t & activity, uint8_t & attribute)
{
	activity = (index >> 8) & 0xFF;
	attribute = index & 0xFF;
}

bool
is_activity_index_valid(file_index_t index)
{
	uint8_t activity, attibute;
	parse_activity_index(index, activity, attibute);
	return (activity < 10) and (attibute < 3);
}

bool
is_file_valid(file_index_t index)
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
