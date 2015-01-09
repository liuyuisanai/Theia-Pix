#pragma once

#include <cstdint>

#include "flash.hpp"

namespace SiKUploader
{

struct Header
{
	uint16_t endian_magic;
	uint16_t version;
	uint16_t record_size;
	uint16_t n_records;
};

struct Record
{
        uint16_t seq;
        uint16_t address;
        uint8_t  size;
        uint8_t  bytes[PROG_MULTI_MAX];

	friend bool
	operator == (const Record & a, const Record & b);
};

struct RecordReader
{
	uint16_t seq, n_records;
	RecordReader() : seq(0), n_records(0) {}

	bool
	restart(int f);

	bool
	read_next(int f, Record & r);

	bool
	done() { return seq == n_records; }
};

bool
is_firmware_file_valid(int f);

} // end of namespace SiKUploader
