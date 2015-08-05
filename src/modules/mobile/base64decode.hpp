#pragma once

#include <cstdint>

#include "buffers.hpp"
#include "io_fragments.hpp"

#include "std_algo.hpp"

namespace base64 {

constexpr size_t
decoded_size(size_t n) { return n / 4 * 3; }

inline uint8_t
decode_6_bits(uint8_t x)
{
    if (x == '/') { return 63; }
    if (x == '+') { return 62; }
    if (x <= '9') { return x - '0' + 52; }
    if (x <= 'Z') { return x - 'A'; }
    return x - 'a' + 26;
}

inline bool
verify_6_bits(uint8_t x)
{
    // It should be like
    // return (('a' <= x) and (x <= 'z'))
    //     or (('A' <= x) and (x <= 'Z'))
    //     or (('0' <= x) and (x <= '9'))
    //     or (x == '/')
    //     or (x == '+');
    //
    // But while ascii is in use:
    uint8_t y = x | 32;
    return (('a' <= y) and (y <= 'z'))
        or (('0' <= x) and (x <= '9'))
        or (x == '/')
        or (x == '+');
}

inline void
decode_8_bits(int16_t x, uint8_t & r1) {
    uint8_t a6 = decode_6_bits(x & 0xff);
    uint8_t b6 = decode_6_bits(x >> 8);
    // char [0]      [1]
    // 76543210 76543210
    // aaaaaabb bbbbcccc
    r1 = (a6 << 2) | (b6 >> 4);
}

inline void
decode_16_bits(int32_t x, uint8_t & r1, uint8_t & r2) {
    uint8_t a6 = decode_6_bits(x & 0xff);
    uint8_t b6 = decode_6_bits((x >> 8) & 0xff);
    uint8_t c6 = decode_6_bits((x >> 16) & 0xff);
    // char [0]      [1]
    // 76543210 76543210
    // aaaaaabb bbbbcccc
    r1 = (a6 << 2) | (b6 >> 4);
    r2 = ((b6 & 0xf) << 4) | (c6 >> 2);
}

inline void
decode_24_bits(int32_t x, uint8_t & r1, uint8_t & r2, uint8_t & r3)
{
    uint8_t a6 = decode_6_bits(x & 0xff);
    uint8_t b6 = decode_6_bits((x >> 8) & 0xff);
    uint8_t c6 = decode_6_bits((x >> 16) & 0xff);
    uint8_t d6 = decode_6_bits(x >> 24);
    // char [0]      [1]      [2]
    // 76543210 76543210 76543210
    // aaaaaabb bbbbcccc ccdddddd
    r1 = (a6 << 2) | (b6 >> 4);
    r2 = ((b6 & 0xf) << 4) | (c6 >> 2);
    r3 = ((c6 & 0x3) << 6) | d6;
}

inline uint8_t *
decode(const uint32_t * data, size_t n, uint8_t * decoded) {
    while (n > 1) {
        decode_24_bits(*data, decoded[0], decoded[1], decoded[2]);
        ++data;
        --n;
        decoded += 3;
    }

    if ((*data >> 24) == '=') {
        if (((*data >> 16) & 0xff) == '=') {
            decode_8_bits(*data, decoded[0]);
            decoded += 1;
        }
        else {
            decode_16_bits(*data, decoded[0], decoded[1]);
            decoded += 2;
        }
    }
    else {
        decode_24_bits(*data, decoded[0], decoded[1], decoded[2]);
        decoded += 3;
    }
    return decoded;
}

inline bool
verify(const uint8_t * data, size_t n)
{
	if (n == 0) { return true; }

	if (n % 4 != 0) { return false; }

	if (data[n - 1] == '=')
	{
		--n;
		if (n == 0) { return false; }
		if (data[n - 1] == '=')
		{
			--n;
			if (n == 0) { return false; }
		}
	}

	return all_of_n(data, n, verify_6_bits);
}

template <size_t ENCODED_BUFFER_BYTES>
struct ReadDecodeWrite {
    static_assert( ENCODED_BUFFER_BYTES % 4 == 0,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
                   " template argument has to be multiple of 4." );
    static_assert( ENCODED_BUFFER_BYTES >= 4,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
                   " template argument has to be at least 4 bytes." );

    constexpr static ssize_t
    BINARY_BUFFER_BYTES = ENCODED_BUFFER_BYTES / 4 * 3;

    ::sink_buffer<ENCODED_BUFFER_BYTES, uint8_t> read_buffer;
    ::source_buffer<BINARY_BUFFER_BYTES, uint8_t> write_buffer;

    template <typename SourceDevice, typename TargetDevice>
    friend bool
    copy(ReadDecodeWrite & self, SourceDevice & sd, TargetDevice & td)
    {
        ssize_t s;
        do
        {
            s = self.refill_read_buffer(sd);
            if (s < 0) { return false; }
            if (self.read_buffer.empty()) { break; }
            self.decode_in_buffer();
            do
            {
                s = self.flush_write_buffer(td);
                if (s < 0) { return false; }
            }
            while (not self.write_buffer.empty());
        }
        while (true);
        return true;
    }

    template <typename SourceDevice>
    inline ssize_t
    refill_read_buffer(SourceDevice & sd)
    {
	read_buffer.reset();
	do {
		ssize_t s = read(sd, read_buffer);
		if (s < 0 and errno != EAGAIN)
		{
			dbg_perror("refill_read_buffer");
			if (read_buffer.size() % 4 == 0) { return s; }
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
		if (s < 0 and errno != EAGAIN)
		{
			dbg_perror("flush_write_buffer");
			return s;
		}
	} while (not write_buffer.empty());
	return r;
    }

    inline void
    decode_in_buffer() {
	uint8_t * decoded_first = write_buffer.begin();
	uint8_t * decoded_last = decode(
		reinterpret_cast<const uint32_t*>(read_buffer.begin()), read_buffer.size() / 4,
		decoded_first);
	write_buffer.mark_tail_used(decoded_last - decoded_first);
    }
};

template <size_t ENCODED_BUFFER_BYTES>
struct ReadVerifyDecodeWrite {
    static_assert( ENCODED_BUFFER_BYTES % 4 == 0,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
                   " template argument has to be multiple of 4." );
    static_assert( ENCODED_BUFFER_BYTES >= 4,
                   "ReadEncodeWrite ENCODED_BUFFER_BYTES"
                   " template argument has to be at least 4 bytes." );

    constexpr static ssize_t
    BINARY_BUFFER_BYTES = ENCODED_BUFFER_BYTES / 4 * 3;

    ::sink_buffer<ENCODED_BUFFER_BYTES, uint8_t> read_buffer;
    ::source_buffer<BINARY_BUFFER_BYTES, uint8_t> write_buffer;

    template <typename SourceDevice, typename TargetDevice>
    friend ssize_t
    copy_verbose(ReadVerifyDecodeWrite & self, SourceDevice & sd, TargetDevice & td)
    {
        ssize_t r = 0;
        while (true)
        {
            ssize_t s = self.refill_read_buffer(sd);
            if (s < 0) { return -1; }
            if (self.read_buffer.empty()) { break; }
            if (not self.verify_buffer()) { return -2; }
            self.decode_in_buffer();
            do
            {
                s = self.flush_write_buffer(td);
                if (s < 0) { return -3; }
		r += s;
            }
            while (not self.write_buffer.empty());
        }
        return r;
    }

    template <typename SourceDevice>
    inline ssize_t
    refill_read_buffer(SourceDevice & sd)
    {
        read_buffer.reset();
        do {
            ssize_t s = read(sd, read_buffer);
            if (s < 0 and errno != EAGAIN)
            {
                dbg_perror("refill_read_buffer");
                if (read_buffer.size() % 4 == 0) { return s; }
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
            if (s < 0 and errno != EAGAIN)
            {
                dbg_perror("flush_write_buffer");
                return s;
            }
        } while (not write_buffer.empty());
        return r;
    }

    inline bool
    verify_buffer() { return verify(read_buffer.begin(), read_buffer.size()); }

    inline void
    decode_in_buffer() {
        uint8_t * decoded_first = write_buffer.begin();
        uint8_t * decoded_last = decode(
            reinterpret_cast<const uint32_t*>(read_buffer.begin()), read_buffer.size() / 4,
            decoded_first);
        write_buffer.mark_tail_used(decoded_last - decoded_first);
    }
};

} // end of namespace base64
