#include <cstring>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "binary_format.hpp"
#include "flash.hpp"

namespace SiKUploader
{

#include "repr.hpp"
#include "serial_config.hpp"

static constexpr int DEV_READ_TIMEOUT_ms = 5000;

static constexpr int8_t PROTO_NOP          = 0x00;
static constexpr int8_t PROTO_OK           = 0x10;
static constexpr int8_t PROTO_FAILED       = 0x11;
static constexpr int8_t PROTO_INSYNC       = 0x12;
static constexpr int8_t PROTO_EOC          = 0x20;
static constexpr int8_t PROTO_GET_SYNC     = 0x21;
static constexpr int8_t PROTO_GET_DEVICE   = 0x22;
static constexpr int8_t PROTO_CHIP_ERASE   = 0x23;
static constexpr int8_t PROTO_LOAD_ADDRESS = 0x24;
static constexpr int8_t PROTO_PROG_FLASH   = 0x25;
static constexpr int8_t PROTO_READ_FLASH   = 0x26;
static constexpr int8_t PROTO_PROG_MULTI   = 0x27;
static constexpr int8_t PROTO_READ_MULTI   = 0x28;
static constexpr int8_t PROTO_PARAM_ERASE  = 0x29;
static constexpr int8_t PROTO_REBOOT	     = 0x30;

bool
serial_config_AT(int d)
{
	bool ok = serial_set_raw(d) and serial_set_speed(d, B57600);
	if (not ok)
	{
		perror("serial_config_AT");
		fprintf(stderr, "Failed termios setup for AT mode.\n");
	}
	fprintf(stderr, "serial_config_AT  %i\n", ok);
	return ok;
}

bool
serial_config_bootloader(int d)
{
	bool ok = serial_set_raw(d) and serial_set_speed(d, B115200);
	if (not ok)
	{
		perror("serial_config_bootloader");
		fprintf(stderr, "Failed termios setup for bootloader mode.\n");
	}
	fprintf(stderr, "serial_config_bootloader %i %i %i\n",
			ok, serial_get_in_speed(d), serial_get_out_speed(d));
	return ok;
}

inline bool
set_blocking_mode(int d, bool blocking)
{
	int flags = fcntl(d, F_GETFL);
	if (blocking)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	int r = fcntl(d, F_SETFL, flags);
	bool ok = r != -1;
	if (not ok)
		perror("set_blocking_mode fcntl");
	return ok;
}

ssize_t
dev_read(int d, void * buf, size_t bufsize)
{
	if (not set_blocking_mode(d, false))
		return -1;

	ssize_t total = 0;
	ssize_t r;

	struct pollfd p;
	p.fd = d;
	p.events = POLLIN;
	do
	{
		p.revents = 0;

		r = poll(&p, 1, DEV_READ_TIMEOUT_ms);
		if (r == -1)
		{
			perror("dev_read / poll");
			return -1;
		}

		if (not p.revents)
			break;

		r = read(d, buf, bufsize);
#ifdef DEBUG_DEV_IO
		int e = errno;
		write(2, "d> ", 3); write_repr(2, buf, r); write(2, "\n", 1);
		errno = e;
#endif // DEBUG_DEV_IO
		if (r == -1)
		{
			perror("dev_read / read");
			return -1;
		}

		bufsize -= r;
		buf += r;
		total += r;
	} while (bufsize > 0);

	return total;
}

ssize_t
dev_write(int d, const void * buf, size_t bufsize)
{
	if (not set_blocking_mode(d, true))
		return -1;
	ssize_t r = write(d, buf, bufsize);
#ifdef DEBUG_DEV_IO
	int e = errno;
	write(2, "<h ", 3); write_repr(2, buf, r); write(2, "\n", 1);
	errno = e;
#endif // DEBUG_DEV_IO
	if (r == -1)
		perror("dev_write / write");
	//else if (tcdrain(d) == 0)
	//	perror("dev_write / tcdrain");
	return r;
}

inline bool
write_string(int d, const char s[])
{
	size_t l = strlen(s);
	if (l == 0) { return true; }

	fprintf(stderr, "h> '%s'\n", s);

	bool ok = dev_write(d, s, l) == l;
	if (not ok)
		perror("write_string");
	return ok;
}

inline bool
expect_string(int d, const char pattern[])
{
	size_t l;
	char buf[64];
	memset(buf, 0, sizeof buf);

	bool ok;
	do
	{
		l = strlen(pattern);
		ok = l == 0;
		if (ok) break;

		fprintf(stderr, "waiting %u bytes\n", l);

		// Wait to receive all the reply
		sleep(1);

		ssize_t s = dev_read(d, buf, sizeof(buf));
		int e = errno;
		fprintf(stderr, "received %i bytes.\n", s);
		fprintf(stderr, "<d '%s'\n", buf);
		ok = s >= (ssize_t)l;
		if (not ok)
		{
			errno = e;
			perror("expect_string");
			break;
		}

		char * c = buf;
		while (*c and *c != *pattern and (c - buf < s))
			++c;
		while (*c and *pattern and *c == *pattern and (c - buf < s))
		{
			++c;
			++pattern;
		}
		ok = c - buf == s and *pattern == '\0';
	} while(false);
	return ok;
}

bool
switch_to_AT_mode(int d)
{
	//write_string(d, "ATO\n");
	//tcflush(d, TCIFLUSH);
	sleep(1);
	bool ok = write_string(d, "+++");
	sleep(1);
	ok = expect_string(d, "OK\r\n")
		and write_string(d, "AT\r\n")
		and expect_string(d, "OK\r\n")
		and write_string(d, "ATI\r\n")
		and expect_string(d, "HM-TRP\r\n");
	return ok;
}

bool
AT_UPDATE(int d)
{
	bool ok = serial_config_AT(d)
		and switch_to_AT_mode(d)
		and write_string(d, "AT&UPDATE\r\n");
	if (not ok)
	{
		fprintf(stderr, "Failed switching to AT&UPDATE.\n");
		return false;
	}

	sleep(1);

	char buf[128];
	read(d, buf, sizeof buf);
	return true;
}

bool
in_sync(int d)
{
	uint8_t buf[2] = {0, 0};
	ssize_t s = dev_read(d, &buf, 2);
	bool ok = s == 2 and buf[0] == PROTO_INSYNC and buf[1] == PROTO_OK;
	if (not ok)
		fprintf(stderr, "in_sync failed: %i 0x%02x 0x%02x\n",
				(int)s, buf[0], buf[1]);
	return ok;
}

bool
command(int d, char c)
{
	char buf[2] = {c, PROTO_EOC};
	size_t s = dev_write(d, buf, 2);
	if (s != 2)
		fprintf(stderr, "command(0x%02x) failed.\n", c);
	return s == 2;
}

bool
identify_board(int d, uint8_t & id)
{
	bool ok = command(d, PROTO_GET_DEVICE);
	if (ok)
	{
		uint8_t info[2] = {0, 0};
		ssize_t s = dev_read(d, info, 2);
		if (s == 2)
			id = info[0];
		else
			fprintf(stderr, "reading GET_DEVICE responce failed.\n");
		ok = s == 2 and in_sync(d);
	}
	return ok;
}

bool
verify_board_id(int d)
{
	uint8_t id;
	bool ok;
	if (not (ok = identify_board(d, id)))
		fprintf(stderr, "Failed reading board id.\n");
	else if (not (ok = id == 0x4e))
		fprintf(stderr, "Invalid board id 0x%02x\n", id);
	else
		fprintf(stderr, "Board id 0x%02x\n", id);
	return ok;
}

bool
load_address(int d, uint16_t address)
{
	uint8_t buf[4] = {
		PROTO_LOAD_ADDRESS,
		(uint8_t)address, (uint8_t)(address >> 8),
		PROTO_EOC
	};
	bool ok = dev_write(d, buf, 4) == 4;
	if (not ok)
		fprintf(stderr, "load_address 0x%04x failed\n", address);
	return ok and in_sync(d);
}

bool
write_flash(int d, const uint8_t bytes[], uint8_t size)
{
	uint8_t buf[2 + PROG_MULTI_MAX + 1] = { PROTO_PROG_MULTI, size };
	memcpy(buf + 2, bytes, size);
	buf[2 + size] = PROTO_EOC;

	return dev_write(d, buf, size + 3) == size + 3 and in_sync(d);
}

bool
write_record(int d, const Record & x)
{
	bool ok = load_address(d, x.address)
		and write_flash(d, x.bytes, x.size);
	if (not ok)
		fprintf(stderr,
			"Failed writing record %i address 0x%04x size %u\n",
			x.seq, x.address, x.size);
	return ok;
}

bool
read_flash(int d, uint8_t bytes[], uint8_t size)
{
	uint8_t cmd[3] = {PROTO_READ_MULTI, size, PROTO_EOC};
	return dev_write(d, cmd, 3) == 3
		and dev_read(d, bytes, size) == size
		and in_sync(d);
}

bool
read_record(int d, uint16_t address, uint8_t size, Record & x)
{
	bool ok = load_address(d, address)
		and read_flash(d, x.bytes, size);
	if (ok)
	{
		x.address = address;
		x.size = size;
	}
	else
	{
		fprintf(stderr, "Failed reading flash at 0x%04x %u\n",
				address, size);
	}
	return ok;
}

bool
verify_record(int d, const Record & x)
{
	Record y { x.seq };
	bool ok = read_record(d, x.address, x.size, y)
		and y == x;
	if (not ok)
		fprintf(stderr, "Verify record %i address 0x%04x %u failed.\n",
				x.seq, x.address, x.size);
	return ok;
}

bool
flash_from_file(int f, int d)
{
	bool ok;
	do
	{
		ok = serial_config_bootloader(d);
		if (not ok)
			break;

		ok = command(d, PROTO_GET_SYNC) and in_sync(d);
		if (not	ok)
		{
			fprintf(stderr, "bootloader sync failed\n");
			break;
		}

		ok = verify_board_id(d)
			and command(d, PROTO_CHIP_ERASE)
			and in_sync(d)
			and command(d, PROTO_PARAM_ERASE)
			and in_sync(d);

		if (not ok)
			break;

		fprintf(stderr, "programming and verifying...\n");

		RecordReader reader;
		reader.restart(f);

		fprintf(stderr, "\n");
		while (ok and not reader.done())
		{
			fprintf(stderr, "\033[2K\r%u of %u. ",
					reader.seq + 1, reader.n_records);
			fflush(stderr);

			Record x;
			ok = reader.read_next(f, x)
				and write_record(d, x)
				and verify_record(d, x);
		}
		fprintf(stderr, "\n");

		ok = command(d, PROTO_REBOOT);
		fprintf(stderr, "rebooting radio.\n");

	}
	while (false);
	return ok;
}

} // end of namespace SiKUploader
