#pragma once

#include "protocol.h"

#include "io_protocol.hpp"
#include "io_util.hpp"
#include "requests.hpp"

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

