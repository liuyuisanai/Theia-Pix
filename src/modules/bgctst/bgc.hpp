#ifndef __BGCTST_BGC_HPP_INCLUDED__
#define __BGCTST_BGC_HPP_INCLUDED__

#include <uORB/topics/frame_button.h>
#include "bgc_uart.hpp"
#include "orb_subscriber.hpp"

namespace BGC {

class BGC {
public:
    // Returns instantly, does not perform any IO.
    BGC();
    
    // Runs the main frame_button -> BGC communication loop, only returns if a fatal error occurs.
    void Run();
    
private:
    enum Poll_result {
          Unknown               = 0
        , Timeout               = (1 << 0)
        , Error                 = (1 << 1)
        , Frame_button_ready    = (1 << 2)
        , BGC_uart_ready        = (1 << 3)
    };
    
private:
    // Initializes frame_button uORB subscription and BGC uart connection, returns true only if everything succeeds.
    bool Setup();
    
    // Polls for any incoming data on frame_button uORB subscription or BGC uart connection.
    Poll_result Poll();
    
private:
    ORB_subscriber frame_button_subscriber;
    BGC_uart bgc_uart;
    
private:
    BGC(const BGC &);
    BGC & operator=(const BGC &);
};

} // namespace BGC

#endif
