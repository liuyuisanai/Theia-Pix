#include "bgc.hpp"

#include <poll.h>
#include "bgc_uart.hpp"
#include "bgc_uart_msg.hpp"
#include "orb_subscriber.hpp"

namespace BGC {

BGC::BGC() : frame_button_subscriber(), bgc_uart() { }

void BGC::Run() {
    if ( !Setup() ) return;
    
    BGC_uart_msg in_msg;
    for ( ; ; ) {
        const Poll_result poll_result = Poll();
        if ( poll_result & Poll_result::Error ) break;
        if ( poll_result & Poll_result::Frame_button_ready ) {
            BGC_uart_msg out_msg;
            struct frame_button_s raw_frame_button_state;
            frame_button_subscriber.Read(ORB_ID(frame_button_state), &raw_frame_button_state);
            switch ( raw_frame_button_state.state ) {
                case SINGLE_CLICK: {
                    DOG_PRINT("[BGC::BGC] Run - SINGLE_CLICK -> SBGC_MENU_CMD_MOTOR_TOGGLE\n");
                    out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_MOTOR_TOGGLE);
                    break;
                }
                case DOUBLE_CLICK: {
                    DOG_PRINT("[BGC::BGC] Run - DOUBLE_CLICK -> SBGC_MENU_CMD_CALIB_ACC\n");
                    out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_ACC);
                    break;
                }
                case TRIPLE_CLICK: {
                    DOG_PRINT("[BGC::BGC] Run - TRIPLE_CLICK -> SBGC_MENU_CMD_CALIB_GYRO\n");
                    out_msg.Build_OUT_CMD_EXECUTE_MENU(SBGC_MENU_CMD_CALIB_GYRO);
                    break;
                }
                case LONG_KEYPRESS: {
                    DOG_PRINT("[BGC::BGC] Run - LONG_KEYPRESS -> ignore\n");
                    break;
                }
                default: {
                    printf("[BGC::BGC] Run - unknown frame button state received\n");
                    break;
                }
            }
            if ( out_msg.Is_fully_valid() && !bgc_uart.Send(out_msg) ) {
                printf("[BGC::BGC] Run - failed to send SBGC_CMD_EXECUTE_MENU\n");
                break;
            }
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
                    } else {
                        printf("[BGC::BGC] Run - received unexpected message\n");
                        in_msg.Dump();
                    }
                }
                in_msg.Reset();
            }
        }
    }
    printf("[BGC::BGC] Run - main loop exited after error\n");
}

bool BGC::Setup() {
    frame_button_subscriber.Open(ORB_ID(frame_button_state));
    if ( !frame_button_subscriber.Is_open() ) return false;
    printf("[BGC::GBC] Setup - subscribed to frame button\n");
    
    bgc_uart.Open();
    if ( !bgc_uart.Is_open() ) return false;
    printf("[BGC::BGC] Setup - opened BGC_uart\n");
    
    int speed = 0, parity = 0;
    bool attributes_discovered = false;
    const int attribute_discovery_tries = 5;
    for ( int try_nr = 0; try_nr < attribute_discovery_tries; ++try_nr ) {
        if ( bgc_uart.Discover_attributes(speed, parity) ) {
            attributes_discovered = true;
            break;
        }
    }
    if ( !attributes_discovered ) {
        printf("[BGC::BGC] Setup - failed to discover BGC_uart attributes\n");
        return false;
    }
    printf("[BGC::BGC] Setup - discovered BGC_uart attributes: speed=%d parity=%d\n", speed, parity);
    
    return true;
}

BGC::Poll_result BGC::Poll() {
    const int timeout_ms = -1;
    const int error_limit = 3;
    for ( int error_count = 0; error_count < error_limit; ++error_count ) {
        struct pollfd pfds[2];
        pfds[0].fd = frame_button_subscriber.Fd(); pfds[0].events = POLLIN;
        pfds[1].fd = bgc_uart.Fd();                pfds[1].events = POLLIN;
        const int poll_ret = poll(pfds, 2, timeout_ms);
        if ( poll_ret < 0 ) {
            printf("[BGC::BGC] Poll - failed to poll: %d\n", errno);
        } else if ( poll_ret == 0 ) {
            return Poll_result::Timeout;
        } else {
            int result = Poll_result::Unknown;
            if ( pfds[0].revents & POLLIN ) result |= Poll_result::Frame_button_ready;
            if ( pfds[1].revents & POLLIN ) result |= Poll_result::BGC_uart_ready;
            return Poll_result(result);
        }
    }
    printf("[BGC::BGC] Poll - error limit reached\n");
    return Poll_result::Error;
}

}
