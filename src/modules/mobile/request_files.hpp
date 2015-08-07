#pragma once

#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include "base64decode.hpp"
#include "base64encode.hpp"
#include "file_indexes.hpp"
#include "read_write_log.hpp"
#include "request_base.hpp"
#include "unique_file.hpp"

#include "std_algo.hpp"


#define WRITE_FILE_TMP_ROOT "/fs/microsd/mobile"
#define WRITE_FILE_TMP_NAME (WRITE_FILE_TMP_ROOT "/tmp.txt")


unique_file
open_by_index(file_index_t file_index, int flags)
{
	filename_buf_t name;
	bool ok = get_filename(file_index, name);
	unique_file f;
	if (ok)
	{
		if (flags & O_CREAT)
		{
			char * last_slash = strrchr(name, '/');
			if (last_slash != nullptr)
			{
				*last_slash = '\0';
				int r = mkdir(name, 0770);
				if (r == -1) { dbg_perror("mkdir(%s)", name); }
				*last_slash = '/';
			}
		}
		f = open(name, flags, 0660);
		if (fileno(f) == -1)
		{
			dbg_perror("open_by_index(0x%08x): %s",
					file_index, name);
		}
	}
	return f;
}

unique_file
open_tmp(int flags)
{
	int r = mkdir(WRITE_FILE_TMP_ROOT, 0660);
	if (r == -1) { dbg_perror("mkdir(" WRITE_FILE_TMP_ROOT ")"); }
	unique_file f = open(WRITE_FILE_TMP_NAME, flags, 0660);
	if (fileno(f) == -1) { dbg_perror("open_tmp"); }
	return f;
}

bool
replace_by_tmp(file_index_t file_index)
{
	filename_buf_t name;
	bool ok = get_filename(file_index, name);
	if (ok)
	{
		/*
		 * It should be enough with rename() here.
		 * But NuttX rename() fails with EEXIST when target exists.
		 * That is why unlink() lives here.
		 */
		int r = unlink(name);
		ok = r == 0;
		if (ok) { rename(WRITE_FILE_TMP_NAME, name); }
		else { dbg_perror("replace_by_tmp(0x%08x)", file_index); }
	}
	return ok;
}

template <>
struct Request< CMD_FILE_INFO >
{
	using value_type = file_index_t;

	static FileInfoReply
	get_fileinfo(file_index_t file_index)
	{
		FileInfoReply r(file_index);
		unique_file f = open_by_index(file_index, O_RDONLY);
		if (fileno(f) != -1)
		{
			r.available = 1;
			r.size = static_cast<file_size_t>(lseek(fileno(f), 0, SEEK_END));
		}
		return r;
	}
};

errcode_t
verify_request(Request< CMD_FILE_INFO >, file_index_t index)
{
	bool ok = is_file_index_valid(index);
	dbg("is_file_index_valid(%u) -> %i.\n", index, ok);
	if (not ok) { return ERRCODE_FILE_INDEX_INVALID; }

	return ERRCODE_OK;
}

template <typename Device>
void
reply(Request< CMD_FILE_INFO >, file_index_t file_index, Device & dev)
{
	auto buf = Request< CMD_FILE_INFO >::get_fileinfo(file_index);
	write(dev, &buf, sizeof buf);
}

template <>
struct Request< CMD_READ_BLOCK >
{
	using value_type = ReadBlockRequest;
	/*
	 * There is no write-only files,
	 * therefore no need to verify read block requests.
	 */
};

template <typename Device>
void
reply(Request< CMD_READ_BLOCK >, ReadBlockRequest req, Device & dev)
{
	unique_file f = open_by_index(req.file_index, O_RDONLY);
	bool ok = fileno(f) != -1;

	if (ok)
	{
		off_t target = req.block_index * FILE_BLOCK_SIZE;
		off_t offset = lseek(fileno(f), 0, SEEK_END);
		ok = target < offset;
		if (ok) {
			offset = lseek(fileno(f), target, SEEK_SET);
			ok = offset == target;
		}
		if (ok) { dbg("read start offset %d\n", (int)offset); }
		else
		{
			if (offset == -1) { dbg_perror("lseek"); }
			else { dbg("Unable to reach offset %i. lseek() -> %i.\n", (int)target, (int)offset); }
		}
	}

	ok = write_char(dev, STX) and ok;

	if (ok)
	{
		DevLog flog { fileno(f), 2, "read file  ", "write file " };
		auto frag = make_fragment< O_RDONLY >( flog, FILE_BLOCK_SIZE );
		base64::ReadEncodeWrite<40 /*bytes buffer*/> b64;
		ok = copy(b64, frag, dev);
		if (not ok) { dbg_perror("base64 encode"); }

		off_t offset = lseek(fileno(f), 0, SEEK_CUR);
		if (offset < 0) { dbg_perror("lseek"); }
		else { dbg("read final offset %d\n", (int)offset); }
	}

	write_char(dev, ETX);
}

struct FileWriteState
{
	/*
	 * Emulation accepts files indexed 1 and 2.
	 * Apply succeeds for index 1.
	 */

	WriteFileRequest request;
	bool request_active;

	FileWriteState() : request_active(false) {}
};

template <>
struct Request< CMD_WRITE_START >
{
	using value_type = WriteFileRequest;
};


template <typename Device>
void
reply(Request< CMD_WRITE_START >, WriteFileRequest file_req, Device &, FileWriteState & state)
{
	/*
	 * There is nothing to reply, but set the state.
	 * verify_request() receives const state by contract.
	 */
	state.request = file_req;
	state.request_active = true;
}

errcode_t
verify_request(Request< CMD_WRITE_START >, WriteFileRequest file_req, const FileWriteState & state)
{
	bool ok = is_file_index_valid(file_req.index);
	if (not ok) { return ERRCODE_FILE_INDEX_INVALID; }

	ok = is_file_writable(file_req.index);
	if (not ok) { return ERRCODE_FILE_READONLY; }

	unique_file f = open_by_index(file_req.index, O_CREAT | O_WRONLY);
	ok = fileno(f) != -1;

	if (ok)
	{
		close(f);

		f = open_tmp(O_CREAT | O_TRUNC | O_WRONLY);
		ok = fileno(f) != -1;
	}

	return ok ? ERRCODE_OK : ERRCODE_FILE_IO_ERROR;
}

template <>
struct Request< CMD_WRITE_BLOCK >
{
	using value_type = WriteBlockRequest;
};

template <typename Device>
void
reply(Request< CMD_WRITE_BLOCK >, WriteBlockRequest, Device &, FileWriteState)
{}

errcode_t
verify_request(Request< CMD_WRITE_BLOCK >, WriteBlockRequest block_req, const FileWriteState & state)
{
	if (not state.request_active) { return ERRCODE_REQUEST_INVALID; }
	if (block_req.file_index != state.request.index) { return ERRCODE_REQUEST_HEADER_INVALID; }
	return ERRCODE_OK;
}

template <typename Device>
inline errcode_t
fetch_expect_char(Device & dev, const char expected)
{
	char received;
	bool ok = read_char_guaranteed(dev, received);
	if (not ok) { return ERRCODE_TIMEOUT; }

	ok = received == expected;
	if (not ok) { return ERRCODE_REQUEST_BODY_INVALID; }

	return ERRCODE_OK;
}

template <typename Device, typename File>
errcode_t
fetch_decode_save_base64(Device & in, file_size_t bin_size, File & out)
{
	file_size_t b64_size = base64::encoded_size(bin_size);
	dbg("size bin %i b64 %i.\n", bin_size, b64_size);
	base64::ReadVerifyDecodeWrite<40 /*bytes buffer*/> b64decode;
	auto src = make_fragment< O_RDONLY >(in, b64_size);
	//auto dst = make_fragment< O_WRONLY >(out, bin_size);
	DevLog log { fileno(out), 2, "read file  ", "write file " };
	auto dst = make_fragment< O_WRONLY >(log, bin_size);

	ssize_t s = copy_verbose(b64decode, src, dst);
	if (s >= 0)
	{
		auto copy_size = static_cast<file_size_t>(s);
		return copy_size == bin_size ? ERRCODE_OK : ERRCODE_TIMEOUT;
	}
	switch (s)
	{
	case -1:
		return ERRCODE_TIMEOUT;
	case -2:
		return ERRCODE_BASE64_ERROR;
	case -3:
		return ERRCODE_FILE_IO_ERROR;
	default:
		return ERRCODE_REQUEST_BODY_INVALID;
	}
}

template <typename Device>
errcode_t
fetch_body(Request< CMD_WRITE_BLOCK >, WriteBlockRequest block_req, Device & dev, FileWriteState & state)
{
	auto block_index = block_req.block_index;

	unique_file f = open_tmp(O_WRONLY);
	if (fileno(f) == -1) { return ERRCODE_FILE_IO_ERROR; }

	off_t size = lseek(fileno(f), 0, SEEK_END);
	if (size == -1)
	{
		dbg_perror("handle_block lseek");
		return ERRCODE_FILE_IO_ERROR;
	}

	if (static_cast<file_size_t>(size) != static_cast<file_size_t>(block_index * FILE_BLOCK_SIZE))
	{
		dbg("Invalid block sequence.\n");
		return ERRCODE_FILE_BLOCK_INVALID;
	}

	/*
	 * Sender is waiting for FF. If the write() fails let there be a
	 * timeout on both sides.
	 */
	write_char(dev, FF);

	errcode_t r = fetch_expect_char(dev, STX);
	if (r != ERRCODE_OK)
	{
		dbg("STX expected.\n");
		return r;
	}

	file_size_t bin_size = min(
		FILE_BLOCK_SIZE,
		state.request.size - block_index * FILE_BLOCK_SIZE
	);
	r = fetch_decode_save_base64(dev, bin_size, f);
	if (r != ERRCODE_OK)
	{
		dbg("fetch_decode_save_base64 error.\n");
		return r;
	}

	r = fetch_expect_char(dev, ETX);
	if (r != ERRCODE_OK) { dbg("ETX expected.\n"); }

	return r;
}

template <>
struct Request< CMD_WRITE_END >
{
	using value_type = file_index_t;
};

template <typename Device>
void
reply(Request< CMD_WRITE_END >, file_index_t, Device &, FileWriteState)
{}

errcode_t
verify_request(Request< CMD_WRITE_END >, file_index_t file_index, FileWriteState & state)
{
	if (not state.request_active) { return ERRCODE_REQUEST_INVALID; }

	/*
	 * verify_request() should receive const state, but here it is avoided
	 * to forget request_active in any case, even it the whole request is
	 * not valid.
	 */
	state.request_active = false;

	if (file_index != state.request.index) { return ERRCODE_REQUEST_HEADER_INVALID; }

	unique_file f = open_tmp(O_RDONLY);
	if (fileno(f) == -1) { return ERRCODE_FILE_IO_ERROR; }

	off_t size = lseek(fileno(f), 0, SEEK_END);
	if (size == -1)
	{
		dbg_perror("handle_block lseek");
		return ERRCODE_FILE_IO_ERROR;
	}

	if (file_size_t(size) != state.request.size) { return ERRCODE_FILE_SIZE_INVALID; }

	close(f);

	bool ok = is_file_content_valid(WRITE_FILE_TMP_NAME, state.request.index);

	ok = replace_by_tmp(state.request.index);
	if (not ok) { return ERRCODE_FILE_IO_ERROR; }

	return ERRCODE_OK;
}
