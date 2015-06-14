#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <systemlib/err.h>
#include <unistd.h>
#include <drivers/drv_hrt.h>
#include <math.h>

#include <uORB/uORB.h>
#include <uORB/topics/debug_data.h>

class debug_data_log{
    public:

        debug_data_log(){

            for (int i=0;i<8;i++)
                debug_data.val[i] = 0.0f;

            debug_data_pub = -1;

        }
       void log(int idx, double value){

            debug_data.val[idx] = value;

            if (debug_data_pub == -1)
                debug_data_pub = orb_advertise(ORB_ID(debug_data), &debug_data);
            else
                orb_publish(ORB_ID(debug_data), debug_data_pub, &debug_data);

        }

    private:

        debug_data_s debug_data;
        int debug_data_pub;
};
