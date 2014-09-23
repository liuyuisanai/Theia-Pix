#pragma once

#define KBD_DEVICE_PATH "/dev/kbd"

#define KBD_SCAN_INTERVAL_usec  16384
#define KBD_SCAN_BUFFER_N_ITEMS 16

using pressed_mask_t = uint16_t;
