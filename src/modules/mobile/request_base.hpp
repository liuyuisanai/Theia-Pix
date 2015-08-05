#pragma once

#include "protocol.h"

template < command_id_t >
struct Request;

template <command_id_t CMD, typename T, typename Device, typename ... Args>
inline errcode_t
fetch_body(Request< CMD >, T, Device &, Args & ...)
{
	dbg("CMD 0x%04x has only header.\n", CMD);
	return ERRCODE_OK;
}

template <command_id_t CMD, typename Device, typename ... Args>
void
reply(Request< CMD >, Device & dev, Args & ...);

template <command_id_t CMD, typename T, typename ... Args>
inline errcode_t
verify_request(Request< CMD >, T, const Args & ...)
{
	dbg("CMD 0x%04x verify_request() default OK.\n", CMD);
	return ERRCODE_OK;
}
