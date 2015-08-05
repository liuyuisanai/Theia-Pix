#pragma once

#include "request_base.hpp"

template <>
struct Request< CMD_HANDSHAKE >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_HANDSHAKE >, Device & dev)
{
	HandshakeReply buf { 0, 0 };
	write(dev, &buf, sizeof buf);
}

template <>
struct Request< CMD_VERSION_FIRMWARE >
{
	using value_type = void;
};

template <typename Device>
void
reply(Request< CMD_VERSION_FIRMWARE >, Device & dev)
{
	VersionFirmwareReply r{ 0, 0 };
	write(dev, &r, sizeof r);
}
