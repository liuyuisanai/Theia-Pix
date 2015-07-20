#ifndef __BGCTST_ORB_SUBSCRIBER_HPP_INCLUDED__
#define __BGCTST_ORB_SUBSCRIBER_HPP_INCLUDED__

#include <errno.h>
#include <stdio.h>
#include <uORB/uORB.h>

namespace BGC {
    class ORB_subscriber {
    public:
        ORB_subscriber() : fd(-1) { }
        
        ORB_subscriber(const struct orb_metadata * const meta) : fd(-1) {
            Open(meta);
        }
        
        bool Open(const struct orb_metadata * const meta) {
            fd = orb_subscribe(meta);
            if ( !Is_open() ) {
                printf("[BGC::ORB_subscriber] failed to subscribe to %s: %d\n", meta->o_name, errno);
                return false;
            }
            return true;
        }
        
        bool Is_open() const {
            return fd >= 0;
        }
        
        void Close() {
            close(fd);
        }
        
        int Fd() {
            return fd;
        }
        
        template<class T>
        void Read(const struct orb_metadata * const meta, T * into) {
            orb_copy(ORB_ID(frame_button_state), fd, into);
        }
        
        ~ORB_subscriber() {
            if ( Is_open() ) Close();
        }
        
    private:
        int fd;
        
    private:
        ORB_subscriber(const ORB_subscriber &);
        ORB_subscriber & operator=(const ORB_subscriber &);
    };
}

#endif
