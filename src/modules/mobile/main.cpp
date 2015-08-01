#include <nuttx/config.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <termios.h>

#include <systemlib/systemlib.h>

#include "protocol.h"

#include "base64decode.hpp"
#include "base64encode.hpp"
#include "file_fragments.hpp"
#include "io_blocking.hpp"
#include "io_protocol.hpp"
#include "io_tty.hpp"
#include "io_util.hpp"
#include "read_write_log.hpp"
#include "status.hpp"
#include "unique_file.hpp"

// Currently in test mode reading input from TELEM2 port
static const char MOBILE_BT_TTY[]="/dev/ttyS2";

static bool daemon_should_run = false;
static bool daemon_running = false;


static const char dummy_filename_path[] = "/fs/microsd/data/";
using filename_buf_t = char[16 + sizeof dummy_filename_path];

static void
get_filename(file_index_t index, filename_buf_t &name)
{
	snprintf(name, sizeof name, "%s/%02u.bin",
			dummy_filename_path, (unsigned)index);
}



template <typename Device>
void
reply_handshake(Device & d) {
	HandshakeReply buf {
		PROTOCOL_VERSION_0,
		DEVICE_KIND_COPTER,
		FIRMWARE_VERSION_DEV_HEAD
	};
	write(d, &buf, sizeof buf);
}

template <typename Device>
void
reply_status_overall(Device & d) {
	static StatusOverall overall;

	StatusOverallReply buf = reply(overall);
	write(d, &buf, sizeof buf);
}

struct FileRequestHandler
{
	template <typename Device>
	bool
	handle(Device & d, command_id_t cmd)
	{
		switch (cmd)
		{
		case COMMAND_FILE_INFO:
			return handle_file_info(d);
		case COMMAND_FILE_BLOCK:
			return handle_file_block(d);
		default:
			// TODO log;
			return false;
		}
	}

	static unique_file
	open_for_read(file_index_t file_index)
	{
		filename_buf_t name;
		get_filename(file_index, name);
		unique_file r = open(name, O_RDONLY);
		if (r.get() == -1) { perror("FileRequestHandler::open_for_read"); fflush(stderr); }
		return r;
	}

	static FileInfoReply
	get_fileinfo(file_index_t file_index)
	{
		struct FileInfoReply r{ file_index };
		unique_file f = open_for_read(file_index);
		if (f.get() != -1)
		{
			r.available = 1;
			r.size = static_cast<file_size_t>(lseek(f.get(), 0, SEEK_END));
		}
		return r;
	}

	template <typename Device>
	bool
	reply_file_info(Device & d, file_index_t file_index)
	{
		FileInfoReply buf = get_fileinfo(file_index);
		ssize_t r = write(d, &buf, sizeof buf);
		bool ok = r != -1;
		if (not ok) { perror("reply_file_info / write"); fflush(stderr); }
		return ok;
	}

	template <typename Device>
	bool
	handle_file_info(Device & d)
	{
		file_index_t buf;
		read_guaranteed(d, &buf, sizeof buf);
		return reply_file_info(d, buf);
	}

	template <typename Device>
	bool
	reply_file_block(Device & d, FileBlockRequest & req)
	{
		unique_file f = open_for_read(req.file_index);
		bool ok = f.get() != -1;

		if (ok)
		{
			off_t target = req.block_index * FILE_BLOCK_SIZE;
			off_t offset = lseek(f.get(), 0, SEEK_END);
			ok = target < offset;
			if (ok) {
				offset = lseek(f.get(), target, SEEK_SET);
				ok = offset == target;
			}
			if (ok) { fprintf(stderr, "read start offset %d\n", (int)offset); fflush(stderr); }
			else
			{
				if (offset == -1) { perror("lseek"); fflush(stderr); }
				else { fprintf(stderr, "Unable to reach offset %i. lseek() -> %i.\n", (int)target, (int)offset); fflush(stderr); }
			}
		}

		ok = ok and write_char(d, STX);

		if (ok)
		{
			DevLog flog { f.get(), 2, "f-read  ", "f-write " };
			auto frag = base64::make_fragment< O_RDONLY >( flog, FILE_BLOCK_SIZE );
			base64::ReadEncodeWrite<40 /*bytes buffer*/> b64;
			ok = copy(b64, frag, d);
			if (not ok) { perror("base64 encode"); fflush(stderr); }

			off_t offset = lseek(f.get(), 0, SEEK_CUR);
			if (offset < 0) { perror("lseek"); fflush(stderr); }
			else { fprintf(stderr, "read final offset %d\n", (int)offset); fflush(stderr); }
		}

		ok = ok and write_char(d, ETX);

		return ok;
	}

	template <typename Device>
	bool
	handle_file_block(Device & d)
	{
		FileBlockRequest buf;
		read_guaranteed(d, &buf, sizeof buf);
		return reply_file_block(d, buf);
	}
};

struct ReceiveFileHandle
{
	/*
	 * Emulation accepts files indexed 1 and 2.
	 * Apply succeeds for index 1.
	 */

	ReceiveFileRequest request;
	bool request_active;

	ReceiveFileHandle() : request_active(false) {}

	template <typename Device>
	bool
	handle(Device & d, command_id_t cmd)
	{
		switch (cmd)
		{
		case COMMAND_RECEIVE_FILE:
			return handle_start(d);
		case COMMAND_RECEIVE_BLOCK:
			return handle_block(d);
		case COMMAND_RECEIVE_APPLY:
			return handle_apply(d);
		default:
			// TODO log;
			return false;
		}
	}

	static unique_file
	open_for_write(file_index_t file_index, int extra_flag=0)
	{
		filename_buf_t name;
		get_filename(file_index, name);
		unique_file r = open(name, O_CREAT | O_WRONLY | extra_flag, 0660);
		if (r.get() == -1) { perror("ReceiveFileHandle::open_for_write"); fflush(stderr); }
		return r;
	}

	template <typename Device>
	bool
	handle_start(Device & d)
	{
		ReceiveFileRequest buf;
		read_guaranteed(d, &buf, sizeof buf);
		bool ok = buf.index == 1 or buf.index == 2;
		if (ok)
		{
			unique_file f = open_for_write(buf.index, O_TRUNC);
			ok = f.get() != -1;
		}
		if (ok) { request = buf; }
		request_active = ok;
		return ok;
	}

	template <typename Device>
	bool
	handle_block(Device & d)
	{
		ReceiveBlockRequest buf;
		read_guaranteed(d, &buf, sizeof buf);
		bool ok = request_active and buf.file_index == request.index;
		unique_file f;
		file_size_t bin_size = 0;
		file_size_t b64_size = 0;
		if (ok)
		{
			f = open_for_write(buf.file_index, O_APPEND);
			ok = f.get() != -1;
		}
		if (ok)
		{
			off_t size = lseek(f.get(), 0, SEEK_END);
			if (size == -1) { perror("ReceiveFileHandle::handle_block lseek"); fflush(stderr); }
			ok = (size >= 0) and
				(size_t(size) == size_t(buf.block_index) * FILE_BLOCK_SIZE);
		}
		if (ok)
		{
			write_char(d, FF);
			char ch;
			ok = read_char_guaranteed(d, ch) and ch == STX;
			if (not ok) { fprintf(stderr, "STX expected"); fflush(stderr); }
		}
		if (ok)
		{
			bin_size = min(
				FILE_BLOCK_SIZE,
				request.size - buf.block_index * FILE_BLOCK_SIZE
			);
			b64_size = base64::encoded_size(bin_size);
			fprintf(stderr, "size bin %i b64 %i.\n", bin_size, b64_size); fflush(stderr);
			base64::ReadVerifyDecodeWrite<40 /*bytes buffer*/> b64decode;
			auto src = base64::make_fragment< O_RDONLY >(d, b64_size);
			//auto dst = base64::make_fragment< O_WRONLY >(f, bin_size);
			DevLog flog { f.get(), 2, "f_read  ", "f_write " };
			auto dst = base64::make_fragment< O_WRONLY >(flog, bin_size);
			ok = copy(b64decode, src, dst);
			if (not ok) { perror("base64 decode"); fflush(stderr); }
		}
		if (ok)
		{
			char ch;
			ok = read_char_guaranteed(d, ch) and ch == ETX;
			if (not ok) { fprintf(stderr, "ETX expected"); fflush(stderr); }
		}
		return ok;
	}

	template <typename Device>
	bool
	handle_apply(Device & d)
	{
		file_index_t buf;
		read_guaranteed(d, &buf, sizeof buf);
		bool ok = request_active and buf == request.index;
		unique_file f;
		if (ok)
		{
			request_active = false;

			f = open_for_write(buf);
			ok = f.get() != -1;
		}
		if (ok)
		{
			off_t size = lseek(f.get(), 0, SEEK_END);
			if (size == -1) { perror("ReceiveFileHandle::handle_block lseek"); fflush(stderr); }
			ok = (size_t)size == request.size;
		}
		return ok;
	}
};

template <typename Device>
void
process_one_command(
	Device & f,
	FileRequestHandler & file_requests,
	ReceiveFileHandle & receive_file
) {
	command_id_t cmd = read_command(f);
	switch (cmd) {
	case COMMAND_BYE:
		// no reply is awaited by the phone
		fprintf(stderr, "BYE received.  No reply sent.\n"); fflush(stderr);
		break;
	case COMMAND_HANDSHAKE:
		reply_ack_command(f, cmd);
		reply_handshake(f);
		break;
	case COMMAND_STATUS_OVERALL:
		reply_ack_command(f, cmd);
		reply_status_overall(f);
		break;
	case COMMAND_FILE_BLOCK:
	case COMMAND_FILE_INFO:
		reply_command_result(f, cmd, file_requests.handle(f, cmd));
		break;
	case COMMAND_RECEIVE_APPLY:
	case COMMAND_RECEIVE_BLOCK:
	case COMMAND_RECEIVE_FILE:
		reply_command_result(f, cmd, receive_file.handle(f, cmd));
		break;
	default:
		reply_nak_command(f, cmd);
		fprintf(stderr, "Unknown command: "); fflush(stderr);
		write_repr(stderr, &cmd, sizeof cmd);
		fprintf(stderr, "\n"); fflush(stderr);
		break;
	}
}

static int
daemon(int argc, char *argv[])
{
	//if (argc != 2) {
	//	fprintf(stderr,"Usage: %s tty\n", argv[0]); fflush(stderr);
	//	return 1;
	//}

	//unique_file d = tty_open(argv[1]);
	unique_file d = tty_open(MOBILE_BT_TTY);
	bool ok = fileno(d) != -1
		and tty_set_speed(fileno(d), B9600)
		and tty_use_ctsrts(fileno(d));
	if (not ok) { return 1; }

	DevLog log (d.get(), 2, "read  ", "write ");
	auto f = make_it_blocking< 1000/*ms*/ >(log);

	daemon_running = true;
	fprintf(stderr, "%s has started.\n", argv[0]);

	FileRequestHandler file_requests;
	ReceiveFileHandle receive_file;

	fprintf(stderr, "Processing");

	while (daemon_should_run)
		process_one_command(f, file_requests, receive_file);

	daemon_running = false;

	fprintf(stderr, "%s has stopped.\n", argv[0]);
	return 0;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{ std::fprintf(stderr, "Usage: %s start|stop|status\n\n", name); }


int
main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "start"))
	{
		if (daemon_running)
		{
			fprintf(stderr, "%s is already running.\n", argv[0]);
			return 1;
		}

		daemon_should_run = true;

		task_spawn_cmd(argv[0],
				SCHED_DEFAULT,
				SCHED_PRIORITY_DEFAULT,
				CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
				daemon,
				argv);
	}
	else if (streq(argv[1], "status"))
	{
		if (daemon_running) { printf("%s is running.\n", argv[0]); }
		else { printf("%s is NOT running.\n", argv[0]); }
	}
	else if (streq(argv[1], "stop"))
	{
		if (not daemon_running)
		{
			fprintf(stderr, "%s is NOT running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = false;
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	fprintf(stderr, "main() is returning 0\n");
	return 0;
}
