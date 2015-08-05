#include <nuttx/config.h>

#include "bgc.hpp"

#include <uORB/topics/frame_button.h>
#include <systemlib/systemlib.h>
#include <poll.h>
#include "bgc_uart.hpp"
#include "bgc_uart_msg.hpp"

namespace BGC {

volatile bool BGC::s_thread_running = false;
volatile bool BGC::s_thread_should_exit = false;

int BGC::s_discovered_speed = -1;
int BGC::s_discovered_parity = -1;

bool BGC::Start_thread() {
    if ( s_thread_running ) {
        printf("[BGC::BGC] Start_thread - already running\n");
        return true;
    }
    
    s_thread_should_exit = false;
    if ( task_spawn_cmd("bgc", SCHED_DEFAULT, SCHED_PRIORITY_DEFAULT, 2000, Thread_main, (const char **)NULL) < 0 ) {
        printf("[BGC::BGC] Start_thread - failed to start thread: %d\n", errno);
        return false;
    }
    
    while ( !s_thread_running && !s_thread_should_exit ) {
        usleep(200);
    }
    printf("[BGC::BGC] Start_thread - started\n");
    
    return true;
}

bool BGC::Stop_thread() {
    if ( !s_thread_running ) {
        printf("[BGC::BGC] Stop_thread - thread not running\n");
        return false;
    }
    
    s_thread_should_exit = true;
    while ( s_thread_running ) {
        usleep(200000);
        printf(".");
    }
    printf("[BGC::BGC] Stop_thread - stopped\n");
    
    return true;
}

int BGC::Thread_main(int argc, char *argv[]) {
    s_thread_running = true;
    BGC bgc;
    if ( !bgc.Initial_setup() ) {
        printf("[BGC::BGC] Thread_main - fatal error, stopping thread\n");
        s_thread_should_exit = true;
    } else {
        for ( ; ; ) {
            if ( !bgc.Run() ) break;
            // Run will return true if it had to quit for some reason, but wants us to Run again.
            // So we sleep for some time, and try again.
            sleep(10);
        }
    }
    s_thread_running = false;
    return 0;
}

BGC::BGC()
    : frame_button_subscriber()
    , vehicle_status_subscriber()
    , bgc_uart()
    , prev_arming_state(arming_state_t(ARMING_STATE_MAX))
    , param_arm_bgc_motors(param_find("A_ARM_BGC_MOTORS"))
{ }

BGC::~BGC() { }

bool BGC::Initial_setup() {
    frame_button_subscriber.Open(ORB_ID(frame_button_state));
    if ( !frame_button_subscriber.Is_open() ) return false;
    printf("[BGC::BGC] Initial_setup - subscribed to frame button\n");
    
    vehicle_status_subscriber.Open(ORB_ID(vehicle_status));
    if ( !vehicle_status_subscriber.Is_open() ) return false;
    if ( !vehicle_status_subscriber.Set_interval(1000) ) return false;
    printf("[BGC::BGC] Initial_setup - subscribed to vehicle status\n");
    
    bgc_uart.Open();
    if ( !bgc_uart.Is_open() ) return false;
    printf("[BGC::BGC] Initial_setup - opened BGC_uart\n");
    
    return true;
}

bool BGC::Run() {
    if ( !Run_setup() ) return !s_thread_should_exit;
    
    BGC_uart_msg in_msg;
    while ( !s_thread_should_exit ) {
        const Poll_result poll_result = Poll();
        if ( poll_result & Poll_result::Error ) return false;
        if ( poll_result & Poll_result::Frame_button_ready ) {
            if ( !Process_frame_button_event() ) break;
        }
        if ( poll_result & Poll_result::Vehicle_status_ready ) {
            if ( !Update_bgc_motor_status() ) break;
        }
        if ( poll_result & Poll_result::BGC_uart_ready ) {
            if ( !bgc_uart.Recv_partial(in_msg) ) break;
            if ( in_msg.Is_fully_present() ) {
                if ( in_msg.Is_fully_valid() ) {
                    if ( in_msg.Command_id() == SBGC_CMD_CONFIRM ) {
                        DOG_PRINT("[BGC::BGC] Run - received SBGC_CMD_CONFIRM\n");
                    } else if ( in_msg.Command_id() == SBGC_CMD_ERROR ) {
                        printf("[BGC::BGC] Run - received SBGC_CMD_ERROR\n");
                        in_msg.Dump();
                    } else if ( in_msg.Command_id() == SBGC_CMD_RESET ) {
                        printf("[BGC::BGC] Run - received SBGC_CMD_RESET, restarting\n");
                        break;
                    } else {
                        printf("[BGC::BGC] Run - received unexpected message\n");
                        in_msg.Dump();
                    }
                }
                in_msg.Reset();
            }
        }
    }
    
    return !s_thread_should_exit;
}

bool BGC::Run_setup() {
    if ( s_discovered_speed == -1 || s_discovered_parity == -1 ) {
        if ( !Discover_attributes() ) return false;
    } else {
        if ( !bgc_uart.Set_attributes(s_discovered_speed, s_discovered_parity) ) return false;
        bool old_attributes_valid = false;
        const int old_attribute_tries = 3;
        for ( int try_nr = 0; try_nr < old_attribute_tries; ++try_nr ) {
            if ( bgc_uart.Get_board_info() ) {
                old_attributes_valid = true;
                break;
            }
        }
        if ( old_attributes_valid ) {
            printf("[BGC::BGC] Run_setup - successfully used old BGC_uart attributes: speed=%d parity=%d\n"
                , s_discovered_speed, s_discovered_parity);
        } else {
            printf("[BGC::BGC] Run_setup - couldn't use old BGC_uart attributes: speed=%d parity=%d\n"
                , s_discovered_speed, s_discovered_parity);
            if ( !Discover_attributes() ) return false;
        }
    }
    
    return true;
}

bool BGC::Discover_attributes() {
    int speed = 0, parity = 0;
    bool attributes_discovered = false;
    const int attribute_discovery_tries = 3;
    for ( int try_nr = 0; try_nr < attribute_discovery_tries; ++try_nr ) {
        if ( s_thread_should_exit ) break;
        if ( bgc_uart.Discover_attributes(speed, parity, s_thread_should_exit) ) {
            attributes_discovered = true;
            break;
        }
    }
    if ( !attributes_discovered ) {
        printf("[BGC::BGC] Discover_attributes - failed to discover BGC_uart attributes\n");
        return false;
    }
    s_discovered_speed = speed;
    s_discovered_parity = parity;
    printf("[BGC::BGC] Discover_attributes - discovered BGC_uart attributes: speed=%d parity=%d\n", speed, parity);
    return true;
}

bool BGC::Process_frame_button_event() {
    BGC_uart_msg out_msg;
    struct frame_button_s raw_frame_button_state;
    if ( !frame_button_subscriber.Read(ORB_ID(frame_button_state), &raw_frame_button_state) ) {
        return false;
    }
    
/** TODO! Implement detection of the DIP-switch that enables button pass-trough. Refer to
 *  https://docs.google.com/document/d/1m2cnf1UrndAgbCF8fEZWD3Evr-s2qAcHdLWIV6tyzqg/edit?usp=sharing
 *  section "Main processor LED and buttons" for more info.
 */
#if BOARD_REVISION < 006
    /** Don't process frame button events on old revisions
     *  that have direct electric link frame_button -> BGC
     */
    printf("[BGC::BGC] Process_frame_button_event - skipping event\n");
    return true;
#endif
    
    switch ( raw_frame_button_state.state ) {
        case SINGLE_CLICK: {
            DOG_PRINT("[BGC::BGC] Process_frame_button_event - SINGLE_CLICK -> SBGC_MENU_CMD_MOTOR_TOGGLE\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_MOTOR_TOGGLE);
            break;
        }
        case DOUBLE_CLICK: {
            DOG_PRINT("[BGC::BGC] Process_frame_button_event - DOUBLE_CLICK -> SBGC_MENU_CMD_CALIB_ACC\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_ACC);
            break;
        }
        case TRIPLE_CLICK: {
            DOG_PRINT("[BGC::BGC] Process_frame_button_event - TRIPLE_CLICK -> SBGC_MENU_CMD_CALIB_GYRO\n");
            out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_GYRO);
            break;
        }
        case LONG_KEYPRESS: {
            DOG_PRINT("[BGC::BGC] Process_frame_button_event - LONG_KEYPRESS -> ignore\n");
            break;
        }
        default: {
            printf("[BGC::BGC] Process_frame_button_event - unknown frame button state received\n");
            break;
        }
    }
    if ( out_msg.Is_first_byte_present() && !bgc_uart.Send(out_msg) ) {
        printf("[BGC::BGC] Process_frame_button_event - failed to send SBGC_CMD_EXECUTE_MENU\n");
        return false;
    }
    return true;
}

bool BGC::Update_bgc_motor_status() {
    BGC_uart_msg out_msg;
    struct vehicle_status_s raw_vehicle_status;
    if ( !vehicle_status_subscriber.Read(ORB_ID(vehicle_status), &raw_vehicle_status) ) {
        return false;
    }
    // DOG_PRINT("[BGC::BGC] Update_bgc_motor_status - change: %d -> %d\n", prev_arming_state, raw_vehicle_status.arming_state);
    if ( prev_arming_state != ARMING_STATE_ARMED && raw_vehicle_status.arming_state == ARMING_STATE_ARMED ) {
        int arm_bgc_motors = 1;
        param_get(param_arm_bgc_motors, &arm_bgc_motors);
        if ( arm_bgc_motors != 0 ) {
            DOG_PRINT("[BGC::BGC] Update_bgc_motor_status - sending CMD_MOTORS_ON\n");
            out_msg.Build_OUT_CMD_MOTORS_ON();
        }
    } else if ( (prev_arming_state == ARMING_STATE_ARMED || prev_arming_state == ARMING_STATE_MAX)
            && raw_vehicle_status.arming_state != ARMING_STATE_ARMED ) {
        int arm_bgc_motors = 1;
        param_get(param_arm_bgc_motors, &arm_bgc_motors);
        if ( arm_bgc_motors != 0 ) {
            DOG_PRINT("[BGC::BGC] Update_bgc_motor_status - sending CMD_MOTORS_OFF\n");
            out_msg.Build_OUT_CMD_MOTORS_OFF();
        }
    }
    prev_arming_state = raw_vehicle_status.arming_state;
    if ( out_msg.Is_first_byte_present() && !bgc_uart.Send(out_msg) ) {
        printf("[BGC::BGC] Run - failed to send CMD_MOTORS_*\n");
        return false;
    }
    return true;
}

BGC::Poll_result BGC::Poll() {
    const int timeout_ms = 1000;
    const int error_limit = 3;
    for ( int error_count = 0; error_count < error_limit; ++error_count ) {
        struct pollfd pfds[3];
        pfds[0].fd = frame_button_subscriber.Fd();   pfds[0].events = POLLIN;
        pfds[1].fd = vehicle_status_subscriber.Fd(); pfds[1].events = POLLIN;
        pfds[2].fd = bgc_uart.Fd();                  pfds[2].events = POLLIN;
        const int poll_ret = poll(pfds, 3, timeout_ms);
        if ( poll_ret < 0 ) {
            printf("[BGC::BGC] Poll - failed to poll: %d\n", errno);
        } else if ( poll_ret == 0 ) {
            return Poll_result::Timeout;
        } else {
            int result = Poll_result::Unknown;
            if ( pfds[0].revents & POLLIN ) result |= Poll_result::Frame_button_ready;
            if ( pfds[1].revents & POLLIN ) result |= Poll_result::Vehicle_status_ready;
            if ( pfds[2].revents & POLLIN ) result |= Poll_result::BGC_uart_ready;
            return Poll_result(result);
        }
    }
    printf("[BGC::BGC] Poll - error limit reached\n");
    return Poll_result::Error;
}

}
