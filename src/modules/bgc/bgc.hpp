#ifndef __BGCTST_BGC_HPP_INCLUDED__
#define __BGCTST_BGC_HPP_INCLUDED__

#include <uORB/topics/vehicle_status.h>
#include <systemlib/param/param.h>
#include "bgc_uart.hpp"
#include "orb_subscriber.hpp"

namespace BGC {

class BGC {
public:
    static bool Start_thread();
    static bool Stop_thread();
    
private:
    // Returns instantly, does not perform any IO.
    BGC();
    
    ~BGC();
    
    // Initializes subscriptions and BGC uart connection, does not detect BGC connection attributes.
    // Returns true only if everything succeeds. Don't run more than once on a single BGC object.
    bool Initial_setup();
    
    // Runs the main frame_button -> BGC communication loop.
    // Returns false if thread was asked to stop, true if an error occurred or BGC restarted.
    bool Run();
    
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
    // Expects Initial_setup to be already done successfully. Detects BGC uart speed/parity attributes.
    bool Run_setup();
    
    // Used during Setup to discover the BGC uart speed/parity attributes.
    bool Discover_attributes();
    
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
    
    param_t param_arm_bgc_motors;
    
private:
    static volatile bool s_thread_running;
    static volatile bool s_thread_should_exit;
    
    static int s_discovered_speed;
    static int s_discovered_parity;
    
    static int Thread_main(int argc, char *argv[]);
    
private:
    BGC(const BGC &);
    BGC & operator=(const BGC &);
};

} // namespace BGC

#endif
