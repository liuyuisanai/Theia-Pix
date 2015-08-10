#pragma once

#include <stdint.h>
#include <stdalign.h>

#ifndef PACKED_STRUCT
#define PACKED_STRUCT	__attribute__((__packed__))
#endif

#define STX			((char)0x02)
#define ETX			((char)0x03)
#define EOT			((char)0x04)
#define ENQ			((char)0x05)
#define ACK			((char)0x06)
#define FF 			((char)0x0C)
#define NAK			((char)0x15)

// command definition helper
#define CMD_AB(c1, c2)          ( ((c2 & 0xff) << 8) | (c1 & 0xff) )

typedef uint16_t command_id_t;
enum {                                              // Little-endian debug
	CMD_HANDSHAKE = CMD_AB('H', 'S'),           // 0x5348
	CMD_BYE = CMD_AB('Y', 'E'),                 // 0x4559
	CMD_VERSION_FIRMWARE = CMD_AB('V', 'F'),    // 0x4656
	CMD_STATUS_OVERALL = CMD_AB('S', 'O'),      // 0x4559
	CMD_ACTIVATION_READ = CMD_AB('A', 'R'),     // 0x5241
	CMD_ACTIVATION_WRITE = CMD_AB('A', 'W'),    // 0x5741
	CMD_FILE_INFO = CMD_AB('F', 'I'),           // 0x4946
	CMD_READ_BLOCK = CMD_AB('R', 'B'),          // 0x4252
	CMD_WRITE_START = CMD_AB('W', 'S'),         // 0x5357
	CMD_WRITE_BLOCK = CMD_AB('W', 'B'),         // 0x4257
	CMD_WRITE_END = CMD_AB('W', 'E')            // 0x4557
};

#undef CMD_AB

typedef char errcode_t;
enum {                                           // Little-endian debug
	ERRCODE_OK = 0x00,
	ERRCODE_TIMEOUT = 0x01,
	ERRCODE_REQUEST_INVALID = 0x02,
	ERRCODE_REQUEST_HEADER_INVALID = 0x03,
	ERRCODE_REQUEST_BODY_INVALID = 0x04,
	ERRCODE_ACTIVATION_FAILED = 'F',         // 0x46
	ERRCODE_BASE64_ERROR = '6',              // 0x36
	ERRCODE_FILE_BLOCK_INVALID = 'B',        // 0x42
	ERRCODE_FILE_INDEX_INVALID = 'I',        // 0x49
	ERRCODE_FILE_SIZE_INVALID = 'S',         // 0x53
	ERRCODE_FILE_IO_ERROR = 'O',             // 0x4f
	ERRCODE_FILE_READONLY = 'R',             // 0x52
	ERRCODE_FILE_WRITE_INACTIVE = 'W',       // 0x57
};

struct PACKED_STRUCT ReplyHeader {
	char ack_nak;
	char ack_err;
	command_id_t cmd;
};

struct PACKED_STRUCT HandshakeReply {
	uint8_t protocol_major;
	uint8_t protocol_minor;
};

struct PACKED_STRUCT VersionFirmwareReply {
	uint8_t major;
	uint8_t minor;
};

struct PACKED_STRUCT StatusOverallReply {
	uint8_t battery_level;
	uint8_t eph;
	uint8_t epv;
	uint8_t error_code;
	uint8_t error_stamp;
};


typedef uint32_t file_index_t;
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

struct PACKED_STRUCT ReadBlockRequest {
	file_index_t file_index;
	file_block_index_t block_index;
};

struct PACKED_STRUCT WriteFileRequest {
	file_index_t index;
	file_size_t size;
};

struct PACKED_STRUCT WriteBlockRequest {
	file_index_t file_index;
	file_block_index_t block_index;
};
