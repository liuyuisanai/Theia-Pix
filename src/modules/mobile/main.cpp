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

ssize_t
write(FILE* f, const void * buf, ssize_t buf_size);

#include "protocol.h"

#include "base64decode.hpp"
#include "base64encode.hpp"
#include "file_fragments.hpp"
#include "io.hpp"
#include "io_tty.hpp"
#include "read_write_log.hpp"
#include "status.hpp"
#include "unique_file.hpp"

#ifdef FORCE_SERIAL_TERMIOS_RAW
# include "serial_config.hpp"
#endif

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



int
open_serial_default(const char name[]) {

	int fd = open(name, O_RDWR | O_NONBLOCK | O_NOCTTY);

    struct termios termAttr;
    tcgetattr(fd, &termAttr);

    cfsetispeed(&termAttr, B57600);

    tty_use_ctsrts(fd);
	if (fd == -1) {
		perror("open");
		std::exit(1);
	}
    return fd;
}


ssize_t
write(FILE* f, const void * buf, ssize_t buf_size) {
	ssize_t r = fwrite(buf, 1, buf_size, f);
	fflush(f);
	return r;
}

template <typename Device>
void
poll_read(Device & dev) {
	struct pollfd p;
	p.fd = fileno(dev);
	p.events = POLLIN;
	p.revents = 0;
	while (poll(&p, 1, 10000) != 1) {}
}

template <typename Device>
ssize_t
read_guaranteed(Device & d, void * buf, size_t buf_size) {
	char * b = static_cast<char *>(buf);
	size_t n = 0;
	do {
		fprintf(stderr, "read() %d of %d.\n", n, buf_size); fflush(stderr);
		ssize_t s = read(d, b + n, buf_size - n);
		if (s >= 0) { n += s; }
		else
		{
			if (errno != EAGAIN) { return s; }
			poll_read(d);
		}
	} while (n < buf_size);
	fprintf(stderr, "read() %d of %d.\n", n, buf_size); fflush(stderr);
	return buf_size;
}

template <typename Device>
bool
read_char_guaranteed(Device & d, char & ch) {
	set_nonblocking_mode(fileno(d));
	ssize_t s;
	do {
		poll_read(d);
		s = read(d, &ch, 1);
	} while (s == -1 and errno == EAGAIN);
	return s == 1;
}

template <typename Device>
void
wait_enq(Device & d) {
	fprintf(stderr, "Waiting ENQ.\n"); fflush(stderr);
	set_nonblocking_mode(fileno(d));
	poll_read(d);
	char ch = 0;
	ssize_t s = read(d, &ch, 1);
	while (ch != ENQ)
	{
		if (s > 0) { fprintf(stderr, "Discarded char %02x\n", (int)ch); fflush(stderr); }
		else { poll_read(d); }
		ch = 0;
		s = read(d, &ch, 1);
	}
}

template <typename Device>
command_id_t
read_command(Device & f) {
	command_id_t r = 0;
	wait_enq(f);
	fprintf(stderr, "Waiting command.\n"); fflush(stderr);
	ssize_t s = read_guaranteed(f, &r, sizeof r);
	if (s < 0) { perror("read_command: read_guaranteed"); fflush(stderr); }
	fprintf(stderr, "Got command: %04x.\n", r); fflush(stderr);
	return r;
}


template <typename Device>
void
write_char(Device & d, char ch) {
	set_blocking_mode(fileno(d));
	write(d, &ch, 1);
}

template <typename Device>
void
reply_command_result(Device & d, const command_id_t cmd, bool r) {
	set_blocking_mode(fileno(d));

	const char ch = r ? ACK : NAK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}

template <typename Device>
void
reply_ack_command(Device & d, const command_id_t cmd) {
	set_blocking_mode(fileno(d));

	const char ch = ACK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}

template <typename Device>
void
reply_nak_command(Device & d, const command_id_t cmd) {
	set_blocking_mode(fileno(d));

	const char ch = NAK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}

template <typename Device>
void
reply_handshake(Device & d) {
	set_blocking_mode(fileno(d));

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
	set_blocking_mode(fileno(d));

	static StatusOverall overall;

	StatusOverallReply buf = reply(overall);
	write(d, &buf, sizeof buf);
}

struct FileRequestHandler
{
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
	void
	reply_file_info(Device & d, file_index_t file_index)
	{
		FileInfoReply buf = get_fileinfo(file_index);
		write(d, &buf, sizeof buf);
	}

	template <typename Device>
	void
	handle_file_info(Device & d)
	{
		file_index_t buf;
		read_guaranteed(d, &buf, sizeof buf);
		reply_file_info(d, buf);
	}

	template <typename Device>
	void
	reply_file_block(Device & d, FileBlockRequest & req)
	{
		char c;
		c = STX;
		write(d, &c, 1);

		unique_file f = open_for_read(req.file_index);
		if (f.get() != -1)
		{
			off_t offset = req.block_index * FILE_BLOCK_SIZE;
			offset = lseek(f.get(), offset, SEEK_SET);
			if (offset < 0) { perror("lseek"); }
			else
			{
				fprintf(stderr, "read start offset %d\n", (int)offset);
				// char buf[ (FILE_BLOCK_SIZE + 2) / 3 * 4 ];
				// ssize_t s = read(f, buf, sizeof(buf));
				// if (s > 0) { write(d, buf, s); }
				DevLog flog  (f.get(), 2 , "", "");
				auto frag = base64::make_fragment<O_RDONLY>(flog, FILE_BLOCK_SIZE);
				base64::ReadEncodeWrite<40 /*bytes buffer*/> b64;
				bool ok = copy(b64, frag, d);
				if (not ok) { perror("base64 encode"); }
				offset = lseek(f.get(), 0, SEEK_CUR);
				if (offset < 0) { perror("lseek"); }
				else { fprintf(stderr, "read final offset %d\n", (int)offset); }
			}
		}

		c = ETX;
		write(d, &c, 1);
	}

	template <typename Device>
	void
	handle_file_block(Device & d)
	{
		FileBlockRequest buf;
		read_guaranteed(d, &buf, sizeof buf);
		reply_file_block(d, buf);
	}

	template <typename Device>
	void
	handle(Device & d, command_id_t cmd)
	{
		switch (cmd)
		{
		case COMMAND_FILE_INFO:
			handle_file_info(d);
			break;
		case COMMAND_FILE_BLOCK:
			handle_file_block(d);
			break;
		// default: TODO log;
		}
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
		reply_ack_command(f, cmd);
		file_requests.handle(f, cmd);
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

	//unique_file d = open_serial_default(argv[1]);
	unique_file d = open_serial_default(MOBILE_BT_TTY);
	DevLog f (d.get(), 2, "read  ", "write ");

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
