#ifndef __BGCTST_BGC_HPP_INCLUDED__
#define __BGCTST_BGC_HPP_INCLUDED__

#include <uORB/topics/frame_button.h>
#include <uORB/topics/vehicle_status.h>
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
        , Vehicle_status_ready  = (1 << 3)
        , BGC_uart_ready        = (1 << 4)
    };
    
private:
    // Initializes frame_button uORB subscription and BGC uart connection, and calls Update_bgc_motor_status(),
    // returns true only if everything succeeds.
    bool Setup();
    
    // Reads current frame button status, sends required command to BGC. Returns true on success, false if something goes wrong.
    bool Process_frame_button_event();
    
    // Reads current vehicle status, if it has changed to/from ARMED since last time, turns BGC motors on/off accordingly.
    // Returns true on success, false if something goes wrong.
    bool Update_bgc_motor_status();
    
    // Polls for any incoming data on frame_button uORB subscription or BGC uart connection.
    Poll_result Poll();
    
private:
    ORB_subscriber frame_button_subscriber;
    ORB_subscriber vehicle_status_subscriber;
    BGC_uart bgc_uart;
    
    // The arming state we read on the previous vehicle status update event.
    arming_state_t prev_arming_state;
    
private:
    BGC(const BGC &);
    BGC & operator=(const BGC &);
};

} // namespace BGC

#endif
