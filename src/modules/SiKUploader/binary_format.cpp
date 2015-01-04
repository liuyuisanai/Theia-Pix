#include <cstdio>
#include <unistd.h>

#include "binary_format.hpp"
#include "flash.hpp"

namespace SiKUploader
{

static constexpr uint16_t MAGIC = 0x137f;
static constexpr uint16_t VERSION = 0;

bool
is_firmware_file_valid(int f)
{
	Header h;

	bool ok = read(f, &h, sizeof h) == sizeof h;
	if (not ok)
		perror("is_firmware_file_valid / read");

	ok = ok and h.endian_magic == MAGIC
		and h.version == VERSION
		and h.record_size == sizeof(Record)
		and h.n_records > 0;

	if (ok)
	{
		off_t size = lseek(f, 0, SEEK_END);
		if (size == -1)
			perror("is_firmware_file_valid / lseek");
		ok = size == sizeof h + h.record_size * h.n_records;
	}

	if (ok)
	{
		RecordReader r;
		ok = r.restart(f);

		while (ok and r.seq < h.n_records)
		{
			Record x;
			ok = r.read_next(f, x);
		}
	}

	return ok;
}

bool
RecordReader::restart(int f)
{
	seq = 0;
	n_records = 0;

	if (lseek(f, 0, SEEK_SET) != 0)
	{
		perror("RecordReader::restart / lseek");
		return false;
	}

	Header h;
	ssize_t s = read(f, &h, sizeof h);
	if (s != sizeof h)
	{
		perror("RecordReader::restart / read");
		return false;
	}

	n_records = h.n_records;

	return true;
}

bool
RecordReader::read_next(int f, Record & r)
{
	ssize_t s = read(f, &r, sizeof r);
	if (s != sizeof r)
	{
		perror("read_record");
		return false;
	}

	if (r.seq != seq and 0 < r.size and r.size <= sizeof r.bytes)
	{
		fprintf(stderr, "Record sequence fails at %u. Found %u.\n",
				seq, r.seq);
		return false;
	}

	++seq;
	return true;
}

bool
operator == (const Record & a, const Record & b)
{
	bool ok = a.seq == b.seq and a.address == b.address
		and a.size == b.size;

	for (uint16_t i = 0; ok and i < a.size; ++i)
		ok = a.bytes[i] == b.bytes[i];

	return ok;
}

} // end of namespace SiKUploader
