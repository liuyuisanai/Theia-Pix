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
                printf("[BGC::ORB_subscriber] failed to orb_subscribe to %s: %d\n", meta->o_name, errno);
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
        bool Read(const struct orb_metadata * const meta, T * into) {
            if ( orb_copy(meta, fd, into) < 0 ) {
                printf("[BGC::ORB_subscriber] failed to orb_copy to %s: %d\n", meta->o_name, errno);
                return false;
            }
            return true;
        }
        
        bool Set_interval(const int interval_ms) {
            if ( orb_set_interval(fd, interval_ms) < 0 ) {
                printf("[BGC::ORB_subscriber] failed to orb_set_interval: %d\n", errno);
                return false;
            }
            return true;
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
