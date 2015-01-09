#pragma once

#include <unistd.h>

#include <cstdlib>
#include <cstring>

// #include <algorithm>
// #include <iterator>
#include <utility>

// #include "string_algorithms.hpp"

namespace AT {

constexpr size_t reply_buffer_size = 512;
constexpr size_t reply_read_retries = 25;
constexpr useconds_t reply_read_delay = 100 * 1000;

// template <typename ForwardIt>
// std::pair<bool, bool> // ok/err received, it was ok
// check_reply_ok_err(ForwardIt first, ForwardIt last) {
// 	bool found = false;
// 	bool ok = false;
// 	if (first != last) {
// 		auto crlf = find_crlf(first, last);
// 		while (crlf != last) {
// 			auto dist = std::distance(first, crlf);
// 			ok = dist == 2 and std::equal(first, crlf, "OK");
// 			found = ok or (dist == 5 and std::equal(first, crlf, "ERROR"));
//
// 			if (found) break;
//
// 			std::advance(crlf, 2);
// 			first = crlf;
// 			crlf = find_crlf(first, last);
// 		}
// 	}
// 	return std::make_pair(found, ok);
// }
//
// template <typename Device>
// std::pair<bool, bool> // ok/err received, it was ok
// wait_ok_error(Device &f) {
// 	auto retries = reply_read_retries;
// 	bool found = false;
// 	bool ok = false;
// 	char buf[reply_buffer_size];
// 	ssize_t s = 0, n = 0;
// 	usleep(reply_read_delay);
// 	while (not found || retries > 0) {
// 		s = read(f, buf + n, sizeof(buf) - n);
// 		if (s < 0) break;
// 		if (s == 0) {
// 			usleep(reply_read_delay);
// 			--retries;
// 			s = read(f, buf + n, sizeof(buf) - n);
// 			if (s <= 0) break;
// 		}
// 		n += s;
// 		std::tie(found, ok) = check_reply_ok_err(buf, buf + n);
// 	}
// 	if (not found and n > 0)
// 		std::tie(found, ok) = check_reply_ok_err(buf, buf + n);
// 	return std::make_pair(found, ok);
// }

template <typename Device>
std::pair<bool, bool> // ok/err received, it was ok
wait_ok_error(Device &f) {
	auto retries = reply_read_retries;
	bool found = false;
	bool ok = false;
	char buf[reply_buffer_size + 1];
	ssize_t s = 0;
       	size_t n = 0;
	usleep(reply_read_delay);
	while (not found || retries > 0) {
		s = read(f, buf + n, reply_buffer_size - n);
		if (s < 0) break;
		if (s == 0) {
			usleep(reply_read_delay);
			--retries;
			s = read(f, buf + n, reply_buffer_size - n);
			if (s <= 0) break;
		}
		n += s;
		buf[n] = '\0';
		ok = std::strstr(buf, "OK\r\n");
		found = ok or std::strstr(buf, "ERROR\r\n");
	}
	if (not found and n > 0) {
		buf[n] = '\0';
		ok = std::strstr(buf, "OK\r\n");
		found = ok or std::strstr(buf, "ERROR\r\n");
	}
	return std::make_pair(found, ok);
}

template <typename Device>
inline bool
write_char(Device &f, char ch) {
	return write(f, &ch, 1) == 1;
}

template <typename Device>
inline bool
exec_cmd(Device &f, const char cmd[], std::size_t len) {
	return write(f, cmd, len) == len and write_char(f, '\r')
		and wait_ok_error(f).second;
}

template <typename Device>
inline bool
exec_cmd(Device &f, const char * const cmd) {
	return exec_cmd(f, cmd, std::strlen(cmd));
}

template <typename Device>
inline bool
exec_reset(Device &f) {
	static const char at_reset[] = "AT*AMWS=0,0,0,0,1,0";
	const ssize_t len = sizeof(at_reset) - 1;
	bool ok = write(f, at_reset, len) == len and write_char(f, '\r');
	if (ok) sleep(1);
	return ok;
}

template <typename Device>
inline bool
switch_to_at_mode(Device &f) {
	sleep(1);
	static const char escape_seq[3] = {'+', '+', '+'};
	bool ok = write(f, escape_seq, sizeof(escape_seq));
	if (ok) {
		sleep(1);
		ok = exec_cmd(f, "AT");
	}
	return ok;
}

template <typename Device>
inline bool
switch_to_data_mode(Device &f) {
	return exec_cmd(f, "AT*ADDM");
}

} // end of namespace AT
