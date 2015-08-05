#pragma once

#include "request_base.hpp"

template <>
struct Request< CMD_ACTIVATION_READ >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_ACTIVATION_READ >, Device & dev)
{
	uint8_t status = 0;
	write(dev, &status, sizeof status);
}

template <>
struct Request< CMD_ACTIVATION_WRITE >
{
	using value_type = uint8_t;
};

inline errcode_t
verify_request(Request< CMD_ACTIVATION_WRITE >, uint8_t level)
{ return ERRCODE_ACTIVATION_FAILED; }

template <typename Device>
void
reply(Request< CMD_ACTIVATION_WRITE >, uint8_t level, Device & dev)
{}
