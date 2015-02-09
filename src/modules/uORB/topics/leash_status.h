#ifndef LEASH_STATUS_H
#define LEASH_STATUS_H

#include "../uORB.h"

struct leash_status_s {
    int8_t menu_mode;
};

/* register this as object request broker structure */
ORB_DECLARE(leash_status);

#endif
