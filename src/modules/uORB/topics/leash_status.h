#ifndef LEASH_STATUS_H
#define LEASH_STATUS_H

#include "../uORB.h"
typedef enum {
    LEASH_MODE_FLY = 0,
    LEASH_MODE_FLY_1 = 1,
    LEASH_MODE_CAM = 2,
}leash_mode;

struct leash_status_s {
    leash_mode menu_mode;
};

/* register this as object request broker structure */
ORB_DECLARE(leash_status);

#endif
