#pragma once

#include "../uORB.h"

/** Be sure to update sdlog defines bellow if you change any of the structures */
/* TODO! [AK] Make an automatic script at least for sdlog types if not names */
struct bt_svc_in_s {
	uint8_t length;
	uint8_t channel;
	uint8_t cmd_evt_id;
	uint8_t flow;
	uint8_t cmd_status;
	uint8_t processed;
};
#define BT_SVC_IN_SDLOG_TYPES "BBBBBB"
#define BT_SVC_IN_SDLOG_NAMES "length,channel,cmd_evt_id,flow,cmd_status,processed"

ORB_DECLARE(bt_svc_in);

struct bt_svc_out_s {
	uint8_t cmd;
	uint8_t ok;
};
#define BT_SVC_OUT_SDLOG_TYPES "BB"
#define BT_SVC_OUT_SDLOG_NAMES "cmd,ok"

ORB_DECLARE(bt_svc_out);

struct bt_evt_status_s {
	uint8_t status;
	uint8_t discoverable_mode;
	uint8_t connectable_mode;
	uint8_t security_mode;
	uint8_t channels_connected;
};
#define BT_EVT_STATUS_SDLOG_TYPES "BBBBB"
#define BT_EVT_STATUS_SDLOG_NAMES "status,discoverable,connectable,security,channels"

ORB_DECLARE(bt_evt_status);

struct bt_link_status_s {
	uint8_t link_quality;
	int8_t rssi;
};
#define BT_LINK_STATUS_SDLOG_TYPES "Bb"
#define BT_LINK_STATUS_SDLOG_NAMES "LinkQuality,RSSI"

ORB_DECLARE(bt_link_status);

//struct bt_channels_s {
//	uint32_t bytes_sent[7];
//	uint32_t bytes_received[7];
//};
//
//ORB_DECLARE(bt_channels);
