#pragma once

#include "../uORB.h"

struct bt_svc_in_s {
	uint8_t length;
	uint8_t channel;
	uint8_t cmd_evt_id;
	uint8_t flow;
	uint8_t cmd_status;
	uint8_t processed;
};

ORB_DECLARE(bt_svc_in);

struct bt_svc_out_s {
	uint8_t cmd;
	uint8_t ok;
};

ORB_DECLARE(bt_svc_out);

struct bt_evt_status_s {
	uint8_t status;
	uint8_t discoverable_mode;
	uint8_t connectable_mode;
	uint8_t security_mode;
	uint8_t channels_connected;
};

ORB_DECLARE(bt_evt_status);

//struct bt_channels_s {
//	uint32_t bytes_sent[7];
//	uint32_t bytes_received[7];
//};
//
//ORB_DECLARE(bt_channels);
