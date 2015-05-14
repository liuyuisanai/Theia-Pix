#pragma once

#include <arpa/inet.h>

namespace BT
{

inline uint16_t
network_to_host(const uint8_t (& bytes)[2])
{
	const uint16_t & p = *(const uint16_t *)bytes;
	return ntohs(p);
}

inline uint32_t
network_to_host(const uint8_t (& bytes)[4])
{
	const uint32_t & p = *(const uint32_t *)bytes;
	return ntohl(p);
}

inline uint32_t
network24_to_host_unsafe(const uint8_t bytes[])
{
	uint32_t a = bytes[0], b = bytes[1], c = bytes[2];
	return (a << 16) + (b << 8) + c;
}

inline uint32_t
network24_to_host(const uint8_t (& bytes)[3])
{ return network24_to_host_unsafe(bytes); }

inline void
host_to_network(uint16_t x, uint8_t (& bytes)[2])
{
	uint16_t *p = (uint16_t*)bytes;
	*p = htons(x);
}

inline void
host_to_network(uint32_t x, uint8_t (& bytes)[4])
{
	uint32_t *p = (uint32_t*)bytes;
	*p = htonl(x);
}

inline void
host24_to_network(uint32_t x, uint8_t (& bytes)[3])
{
	bytes[0] = x >> 16;
	bytes[1] = x >> 8;
	bytes[2] = x;
}

}
// end of namespace BT
