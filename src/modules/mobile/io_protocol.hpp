#pragma once

#include "io_util.hpp"

template <typename Device>
void
wait_enq(Device & d) {
	fprintf(stderr, "Waiting ENQ.\n"); fflush(stderr);
	char ch = 0;
	ssize_t s = read(d, &ch, 1);
	while (ch != ENQ)
	{
		if (s > 0) { fprintf(stderr, "Discarded char %02x\n", (int)ch); fflush(stderr); }
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
reply_command_result(Device & d, const command_id_t cmd, bool r) {
	const char ch = r ? ACK : NAK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}

template <typename Device>
void
reply_ack_command(Device & d, const command_id_t cmd) {
	const char ch = ACK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}

template <typename Device>
void
reply_nak_command(Device & d, const command_id_t cmd) {
	const char ch = NAK;
	write(d, &ch, 1);
	write(d, &cmd, sizeof cmd);
}
