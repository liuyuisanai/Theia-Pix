#include <cstdint>

#include "buffers.hpp"

namespace base64 {

constexpr size_t
encoded_size(size_t n) { return (n + 2) / 3 * 4; }

uint8_t
encode_6_bits(uint8_t x)
{
    if (x < 26) { return 'A' + x; }
    x -= 26;
    if (x < 26) { return 'a' + x; }
    x -= 26;
    if (x < 10) { return '0' + x; }
    x -= 10;
    if (x == 0) { return '+'; }
    return '/';
}

uint32_t
encode_8_bits(uint8_t x0)
{
    // char [0]
    // 76543210
    // aaaaaabb
    uint32_t r = encode_6_bits(x0 >> 2);
    r |= encode_6_bits((x0 & 0x3) << 4) << 8;
    r |= '=' << 16;
    r |= '=' << 24;
    return r;
}

uint32_t
encode_16_bits(uint8_t x0, uint8_t x1)
{
    // char [0]      [1]
    // 76543210 76543210
    // aaaaaabb bbbbcccc
    uint32_t r = encode_6_bits(x0 >> 2);
    r |= encode_6_bits(((x0 & 0x3) << 4) | (x1 >> 4)) << 8;
    r |= encode_6_bits((x1 & 0xf) << 2) << 16;
    r |= '=' << 24;
    return r;
}

uint32_t
encode_24_bits(uint8_t x0, uint8_t x1, uint8_t x2)
{
    // char [0]      [1]      [2]
    // 76543210 76543210 76543210
    // aaaaaabb bbbbcccc ccdddddd
    uint32_t r = encode_6_bits(x0 >> 2);
    r |= encode_6_bits(((x0 & 0x3) << 4) | (x1 >> 4)) << 8;
    r |= encode_6_bits(((x1 & 0xf) << 2) | (x2 >> 6)) << 16;
    r |= encode_6_bits(x2 & 0x3f) << 24;
    return r;
}

uint32_t *
encode(const uint8_t * data, size_t n, uint32_t * encoded)
{
    while (n > 2) {
        *encoded = encode_24_bits(data[0], data[1], data[2]);
        ++encoded;
        data += 3;
        n -= 3;
    }
    switch(n) {
    case 2:
        *encoded = encode_16_bits(data[0], data[1]);
        ++encoded;
        break;
    case 1:
        *encoded = encode_8_bits(data[0]);
        ++encoded;
        break;
    }
    return encoded;
}

struct FragmentReader
{
	size_t bytes_left;
	FragmentReader(size_t size) : bytes_left{ size } {}

	template <typename Device>
	friend inline ssize_t
	read(FragmentReader & self, Device & d, void * buf, size_t buf_size)
	{
		if (self.bytes_left < buf_size) { buf_size = self.bytes_left; }
		ssize_t s = read(d, buf, buf_size);
		if (s > 0) { self.bytes_left -= s; }
		return s;
	}
};

template <typename Device>
struct DeviceFragment
{
	Device * dev;
	FragmentReader fr;

	DeviceFragment(Device * d, size_t fragment_size)
	: dev{ d }, fr{ fragment_size } {}

	friend inline ssize_t
	read(DeviceFragment & self, void * buf, size_t buf_size)
	{ return read(self.fr, *self.dev, buf, buf_size); }
};

template <size_t ENCODED_BUFFER_BYTES>
struct ReadEncodeWrite {
    static_assert( ENCODED_BUFFER_BYTES % 4 == 0,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
		   " template argument has to be multiple of 4." );
    static_assert( ENCODED_BUFFER_BYTES > 4,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
		   " template argument has to be at least 4 bytes." );

    constexpr static ssize_t BINARY_BUFFER_BYTES = ENCODED_BUFFER_BYTES / 4 * 3;

    ::sink_buffer<BINARY_BUFFER_BYTES, uint8_t> read_buffer;
    ::source_buffer<ENCODED_BUFFER_BYTES, uint8_t> write_buffer;

    template <typename SourceDevice, typename TargetDevice>
    friend bool
    copy(ReadEncodeWrite & self, SourceDevice & sd, TargetDevice & td)
    {
	ssize_t s;
	do {
		s = self.refill_read_buffer(sd);
		if (s < 0) { return false; }
		if (self.read_buffer.empty()) { break; }
		self.encode_in_buffer();
		s = self.flush_write_buffer(td);
		if (s < 0) { return false; }
		if (not self.read_buffer.full()) { break; }
	} while (true);
	return true;
    }

    template <typename SourceDevice>
    inline ssize_t
    refill_read_buffer(SourceDevice & sd)
    {
	read_buffer.reset();
	do {
		ssize_t s = read(sd, read_buffer);
		if (s < 0)
		{
			if (errno != EAGAIN) { return s; }
		}
		if (s == 0) { break; }
	} while (not read_buffer.full());
	return read_buffer.size();
    }

    template <typename TargetDevice>
    inline ssize_t
    flush_write_buffer(TargetDevice & td)
    {
	ssize_t r = write_buffer.size();
	do {
		ssize_t s = write(td, write_buffer);
		if (s < 0)
		{
			if (errno != EAGAIN) { return s; }
		}
	} while (not write_buffer.empty());
	return r;
    }

    inline void
    encode_in_buffer() {
	uint8_t * encoded_first = write_buffer.begin();
	uint8_t * encoded_last = reinterpret_cast<uint8_t*>(encode(
		reinterpret_cast<const uint8_t*>(read_buffer.begin()), read_buffer.size(),
		reinterpret_cast<uint32_t*>(encoded_first)));
	write_buffer.mark_tail_used(encoded_last - encoded_first);
    }
};

} // end of namespace base64
