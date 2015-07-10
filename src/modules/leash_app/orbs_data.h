#include <poll.h>

#include "uORB/topics/kbd_handler.h"
#include "uORB/topics/airdog_status.h"

struct SubFd {
    const struct orb_metadata meta;
    const struct pollfd file_descripter;
};

class OrbsData {
    public:
        OrbsData() {};
        ~OrbsData() {};
        void get_data(struct orb_metadata * meta);
    private:
        uint16_t num_of_subs;
        SubFd subscribtions[num_of_subs];
};

namespace leash_orbs{

static struct OrbsSubs sublist;
static struct pollfd fds[2];

void fds_init() {
    sublist.button_sub = orb_subscribe(ORB_ID(kbd_handler));
    sublist.airdog_status_sub = orb_subscribe(ORB_ID(airdog_status));

    fds[0].fd = sublist.button_sub;
    fds[0].events = POLLIN;
    fds[1].fd = sublist.airdog_status_sub;
    fds[1].events = POLLIN;
}

} //end of namespace leash_orbs
