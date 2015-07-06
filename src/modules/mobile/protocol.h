#pragma once

#include <stdint.h>
#include <stdalign.h>

#ifndef PACKED_STRUCT
#define PACKED_STRUCT	__attribute__((__packed__))
#endif

#define STX			0x02
#define ETX			0x03
#define EOT			0x04
#define ENQ			0x05
#define ACK			0x06
#define FF 			0x0B
#define NAK			0x15

// command definition helper
#define CMD_AB(c1, c2)          ( ((c2 & 0xff) << 8) | (c1 & 0xff) )

typedef uint16_t command_id_t;
enum {
	COMMAND_HANDSHAKE = CMD_AB('H', 'S'),
	COMMAND_BYE = CMD_AB('Y', 'E'),
	COMMAND_STATUS_OVERALL = CMD_AB('S', 'O'),
	COMMAND_FILE_INFO = CMD_AB('F', 'I'),
	COMMAND_FILE_BLOCK = CMD_AB('F', 'B'),
	COMMAND_RECEIVE_FILE = CMD_AB('v', 'f'),
	COMMAND_RECEIVE_BLOCK = CMD_AB('v', 'b'),
	COMMAND_RECEIVE_APPLY = CMD_AB('v', 'a'),
};

#undef CMD_AB

//
// COMMAND_HANDSHAKE reply
//
typedef uint16_t protocol_version_id_t;
enum {
	PROTOCOL_VERSION_0
};

typedef uint8_t device_kind_t;
enum {
	DEVICE_KIND_COPTER,
	DEVICE_KIND_REMOTE_CONTROL
};

typedef uint8_t firmware_version_t;
enum {
	FIRMWARE_VERSION_DEV_HEAD
};

struct PACKED_STRUCT HandshakeReply {
	protocol_version_id_t protocol_version;
	device_kind_t device_kind;
	firmware_version_t firmware_version;
};

struct PACKED_STRUCT StatusOverallReply {
	uint8_t gps_error_horizontal;
	uint8_t gps_error_vertical;
	uint8_t battery_level;
};

typedef uint8_t file_index_t;
typedef uint32_t file_size_t;
#define FILE_BLOCK_SIZE ((file_size_t)1024)
typedef uint32_t file_block_index_t;

struct PACKED_STRUCT FileInfoReply {
	file_index_t index;
	uint8_t	available;
	file_size_t size;
#ifdef __cplusplus
	FileInfoReply(file_index_t n) : index(n), available(0), size(0) {}
	FileInfoReply & operator=(FileInfoReply const &) = default;
#endif
};

struct PACKED_STRUCT FileBlockRequest {
	file_index_t file_index;
	file_block_index_t block_index;
};

struct PACKED_STRUCT ReceiveFileRequest {
	file_index_t index;
	file_size_t size;
};

struct PACKED_STRUCT ReceiveBlockRequest {
	file_index_t file_index;
	file_block_index_t block_index;
};
