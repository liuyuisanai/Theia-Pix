#pragma once

#include "debug.hpp"
#include "requests.hpp"
#include "io_protocol.hpp"

template < command_id_t CMD, typename ParamT >
struct RequestData
{
	ParamT x;
};

template < command_id_t CMD, typename T, typename Device, typename ... Args >
inline errcode_t
fetch(RequestData< CMD, T > & self, Device & dev, Args & ... args)
{
	Request< CMD > tag;
	ssize_t s = read_guaranteed(dev, &self.x, sizeof self.x);
	errcode_t r;
	if (s == sizeof self.x)
	{
		const auto & const_self = self;
		r = verify_request(tag, const_self.x, args...);
		dbg("CMD 0x%04x fetch: verify_request() -> 0x%02x.\n", CMD, r);
		if (r == ERRCODE_OK) { r = fetch_body(tag, self.x, dev, args...); }
	}
	else
	{
		if (s == -1) { dbg_perror("fetch: read"); }
		else { dbg("CMD 0x%04x fetch: insufficient parameters.\n", CMD); }
		r = ERRCODE_TIMEOUT;
	}
	return r;
}

template < command_id_t CMD, typename T, typename Device, typename ... Args >
void
reply(RequestData<CMD, T> & self, Device & dev, Args & ... args)
{
	Request< CMD > tag;
	reply(tag, self.x, dev, args...);
}

template <command_id_t CMD >
struct RequestData< CMD, void >
{};

template < command_id_t CMD, typename Device, typename ... Args >
inline errcode_t
fetch(RequestData< CMD, void > & self, Device & dev, Args & ... args)
{ return ERRCODE_OK; }

template < command_id_t CMD, typename Device, typename ... Args >
void
reply(RequestData<CMD, void> & self, Device & dev, Args & ... args)
{
	Request< CMD > tag;
	reply(tag, dev, args...);
}

template <command_id_t CMD>
using request_data_type = RequestData< CMD, typename Request< CMD >::value_type >;

template < command_id_t CMD, typename Device, typename ... Args >
void
handle(Device & dev, Args & ... args)
{
	request_data_type< CMD > req;
	errcode_t r = fetch(req, dev, args...);
	fprintf(stderr, "CMD 0x%04x error code 0x%02x.\n", CMD, r);
	reply_command_result(dev, r, CMD);
	if (r == ERRCODE_OK) { reply(req, dev, args...); }
}

void
dump_unknown_command(command_id_t cmd)
{
	static_assert(
		sizeof cmd == 2,
		"dump_unknown_command() depends on command_id_t size"
	);
	uint8_t * p = reinterpret_cast<uint8_t*>(&cmd);
	dbg("Unknown command: %02x %02x\n", p[0], p[1]);
}

template <typename Device>
void
process_one_command(
	Device & f,
	FileWriteState & receive_file,
	const StatusOverall & status
) {
	command_id_t cmd;
	bool ok = wait_enq(f);
	if (not ok) { return; }

	ok = read_command(f, cmd);
	if (not ok)
	{
		reply_command_result(f, ERRCODE_TIMEOUT, cmd);
		dump_unknown_command(cmd);
		return;
	}
	switch (cmd) {
	case CMD_BYE:
		/*
		 * No reply is awaited by the phone.
		 */
		dbg("BYE received.  No reply sent.\n");
		break;
	case CMD_HANDSHAKE:
		handle< CMD_HANDSHAKE >(f);
		break;
	case CMD_VERSION_FIRMWARE:
		handle< CMD_VERSION_FIRMWARE >(f);
		break;
	case CMD_STATUS_OVERALL:
		handle< CMD_STATUS_OVERALL >(f, status);
		break;
	case CMD_ACTIVATION_READ:
		handle< CMD_ACTIVATION_READ >(f);
		break;
	case CMD_ACTIVATION_WRITE:
		handle< CMD_ACTIVATION_WRITE >(f);
		break;
	case CMD_FILE_INFO:
		handle< CMD_FILE_INFO >(f);
		break;
	case CMD_READ_BLOCK:
		handle< CMD_READ_BLOCK >(f);
		break;
	case CMD_WRITE_START:
		handle< CMD_WRITE_START >(f, receive_file);
		break;
	case CMD_WRITE_BLOCK:
		handle< CMD_WRITE_BLOCK >(f, receive_file);
		break;
	case CMD_WRITE_END:
		handle< CMD_WRITE_END >(f, receive_file);
		break;
	default:
		reply_command_result(f, ERRCODE_REQUEST_INVALID, cmd);
		dump_unknown_command(cmd);
		break;
	}
}
