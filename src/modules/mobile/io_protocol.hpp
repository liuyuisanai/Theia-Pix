#pragma once

#include "protocol.h"
#include "io_util.hpp"

template <typename Device>
bool
wait_enq(Device & d) {
	dbg("Waiting ENQ.\n");
	char ch = 0;
	ssize_t s = read(d, &ch, 1);
	while (ch != ENQ)
	{
		if (s == -1)
		{
			if (errno != ETIMEDOUT) { dbg_perror("wait_enq"); }
			return false;
		}
		if (s > 0) { dbg("Discarded char %02x\n", (int)ch); }
		ch = 0;
		s = read(d, &ch, 1);
	}
	return true;
}

template <typename Device>
bool
read_command(Device & f, command_id_t & cmd) {
	dbg("Waiting command.\n");
	ssize_t s = read_guaranteed(f, &cmd, sizeof cmd);
	if (s < 0) { dbg_perror("read_command: read_guaranteed"); }
	dbg("Got command: %04x.\n", cmd);
	return s == sizeof cmd;
}

template <typename Device>
void
reply_command_result(Device & d, const errcode_t err, const command_id_t cmd) {
	const ReplyHeader r
	{
		err == ERRCODE_OK ? ACK : NAK,
		err == ERRCODE_OK ? ACK : err,
		cmd
	};
	write(d, &r, sizeof r);
}
