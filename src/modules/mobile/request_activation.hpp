#pragma once

#include <airdog/activation.hpp>

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
	using namespace AirDog;
	uint8_t status = activation::get();
	write(dev, &status, sizeof status);
}

template <>
struct Request< CMD_ACTIVATION_WRITE >
{
	using value_type = uint8_t;
};

inline errcode_t
verify_request(Request< CMD_ACTIVATION_WRITE >, uint8_t level)
{
	using namespace AirDog;
	bool ok = activation::set_store(level);
	return ok ? ERRCODE_OK : ERRCODE_ACTIVATION_FAILED;
}

template <typename Device>
void
reply(Request< CMD_ACTIVATION_WRITE >, uint8_t level, Device & dev)
{}
