#include <nuttx/config.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <errno.h>
#include <cstdlib>
#include <ctime>

#include <termios.h>


#include "protocol.h"

#include "io.hpp"
#include "read_write_log.hpp"
#include "unique_file.hpp"
#include "base64encode.hpp"
#include "io_tty.hpp"

#include <systemlib/systemlib.h>

// Currently in test mode reading input from TELEM2 port
const char MOBILE_BT_TTY[]="/dev/ttyS2";
int cnt = 0;

int
open_serial_default(const char name[]);

ssize_t
write(FILE* f, const void * buf, ssize_t buf_size);

static bool daemon_should_run = false;
static bool daemon_running = false;


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
		ssize_t s = read(d, b + n, buf_size - n);
		if (s >= 0) { n += s; }
		else
		{
			if (errno != EAGAIN) { return s; }
			poll_read(d);
		}
	} while (n < buf_size);
	return buf_size;
}

template <typename Device>
void
wait_enq(Device & d) {
	fprintf(stderr, "Waiting ENQ.\n");
	set_nonblocking_mode(fileno(d));
    char ch = 0;
    // while (1) {
    //
	//     poll_read(d);
    //
    //     ch = 0;
    //     ssize_t s = read(d, &ch, 1);
    //
    //     fprintf(stderr, "%d %d\n", s, errno);
    //
    //     for (int i=0;i<8;i++)
    //         if ((ch>>i)&1)
    //             fprintf(stderr, "1");
    //         else
    //             fprintf(stderr, "0");
    //
    //     fprintf(stderr, "\n");
    //
    // }

    ssize_t s = read(d, &ch, 1);
    
	while (ch != ENQ)
	{
		if (s > 0) { fprintf(stderr, "Discarded char %02x\n", (int)ch); }
		else poll_read(d); 
		ch = 0;
		s = read(d, &ch, 1);
	}

}

template <typename Device>
command_id_t
read_command(Device & f) {
	command_id_t r = 0;
	wait_enq(f);
	fprintf(stderr, "Waiting command.\n");
	ssize_t s = read_guaranteed(f, &r, sizeof(r));
	if (s < 0) { perror("read_command: read_guaranteed"); }
	return r;
}


template <typename Device>
void
reply_ack_command(Device & d, const command_id_t cmd) {
	set_blocking_mode(fileno(d));

	const char ch = ACK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof(cmd));
}

template <typename Device>
void
reply_nak_command(Device & d, const command_id_t cmd) {
	set_blocking_mode(fileno(d));

	const char ch = NAK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof(cmd));
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
	write(d, &buf, sizeof(buf));
}

template <typename Device>
void
reply_status_overall(Device & d) {
	set_blocking_mode(fileno(d));

	StatusOverallReply buf {5, 10, 255};
	write(d, &buf, sizeof(buf));
}

template <typename Device>
void
receive_presets(Device & d) {
	set_blocking_mode(fileno(d));

	fprintf(stderr, "Starting receiving presets' parameters.\n");
	do {
		char ch = 0;
		read(d, &ch, 1);
		if (ch == RecordSeparator) {
			PresetParameter p;
			memset(&p, 0, sizeof(p));
			ssize_t s = read_guaranteed(d, &p, sizeof(p));
			fprintf(stderr, "read -> %d\n", (int)s);
			printf("Preset %u parameter %u: ",
					(unsigned)p.preset_id,
					(unsigned)p.parameter_id);
			write_repr(stdout, p.value, sizeof(p.value));
			printf("\n");
			const char ch = ACK;
			write(d, &ch, 1);
		}
		else if (ch == EOT) {
			fprintf(stderr, "Presets' parameters should be applied.\n");
			const char ch = ACK;
			write(d, &ch, 1);
			break;
		}
		else if (ch == CAN) {
			fprintf(stderr, "Presets' parameters setting cancelled.\n");
			const char ch = ACK;
			write(d, &ch, 1);
			break;
		}
		else {
			fprintf(stderr, "Presets' parameters protocol error.\n");
			const char ch = NAK;
			write(d, &ch, 1);
			break;
		}
	} while (true);

	fprintf(stderr, "Stopped receiving presets' parameters.\n");
}

struct FileRequestHandler
{
	using filename_buf_t = char[6 + sizeof(file_index_t) * 2];

	static void
	get_filename(file_index_t index, filename_buf_t &name)
	{ snprintf(name, sizeof(name), "data/%02x", (unsigned)index); }

	static unique_file
	open_for_read(file_index_t file_index)
	{
		filename_buf_t name;
		get_filename(file_index, name);
		unique_file r = open(name, O_RDONLY);
		if (r.get() == -1) { perror("FileRequestHandler::open_for_read"); }
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
		write(d, &buf, sizeof(buf));
	}

	template <typename Device>
	void
	handle_file_info(Device & d)
	{
		file_index_t buf;
		read_guaranteed(d, &buf, sizeof(buf));
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
				base64::DeviceFragment<DevLog> frag{ &flog, FILE_BLOCK_SIZE };
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
		read_guaranteed(d, &buf, sizeof(buf));
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

template <typename Device>
void
process_one_command(Device & f, FileRequestHandler & file_requests)
{
    fprintf(stderr, "Processing");

	command_id_t cmd = read_command(f);
	switch (cmd) {
	case COMMAND_BYE:
		// no reply is awaited by the phone
		fprintf(stderr, "BYE received.  No reply sent.\n");
		break;
	case COMMAND_HANDSHAKE:
		reply_ack_command(f, cmd);
		reply_handshake(f);
		break;
	case COMMAND_STATUS_OVERALL:
		reply_ack_command(f, cmd);
		reply_status_overall(f);
		break;
	case COMMAND_RECEIVE_PRESET:
		reply_ack_command(f, cmd);
		receive_presets(f);
		break;
	case COMMAND_FILE_INFO:
		reply_ack_command(f, cmd);
		file_requests.handle(f, cmd);
		break;
	case COMMAND_FILE_BLOCK:
		reply_ack_command(f, cmd);
		file_requests.handle(f, cmd);
		break;
	default:
		reply_nak_command(f, cmd);
		fprintf(stderr, "Unknown command: ");
		//write_repr(stderr, &cmd, sizeof(cmd));
		fprintf(stderr, "\n");
		break;
	}
}

static int
daemon(int argc, char *argv[])
{

	daemon_running = true;
	fprintf(stderr, "%s has started.\n", argv[0]);

	unique_file d = open_serial_default(MOBILE_BT_TTY);
    DevLog f(d.get(), 2 , "", "");

	FileRequestHandler file_requests;

    fprintf(stderr, "Processing");

	while (daemon_should_run)
		process_one_command(f, file_requests);

	daemon_running = false;

	fprintf(stderr, "%s has stopped.\n", argv[0]);
	return 0;

}

static inline bool
streq(const char a[], const char b[]) 
{ 
    return std::strcmp(a, b) == 0; 
}

static void
usage(const char name[])
{ 
    std::fprintf(stderr, "Usage: %s start|stop|status\n\n", name); 
}


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
