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

}
// end of namespace BT
