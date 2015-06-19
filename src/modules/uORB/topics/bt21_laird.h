#ifndef BT_DEBUG_H_
#define BT_DEBUG_H_

#include "../uORB.h"

struct bt_command_s {
	uint8_t cmd;
	uint8_t result;
};

ORB_DECLARE(bt_command);

struct bt_event_s {
	uint8_t event;
};

ORB_DECLARE(bt_event);

struct bt_status_s {
	uint8_t connectable;
	uint8_t discoverable;
	uint8_t service_status;
};

ORB_DECLARE(bt_status);

struct bt_channels_s {
	uint32_t bytes_sent[7];
	uint32_t bytes_received[7];
};

ORB_DECLARE(bt_channels);

#endif // BT_DEBUG_H_
