#include "bgc_uart.hpp"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include "bgc_uart_msg.hpp"

namespace BGC {

BGC_uart::BGC_uart() : fd(-1) { }

BGC_uart::BGC_uart(const char * const port, const int speed, const int parity) : fd(-1) {
    Open(port, speed, parity);
}

bool BGC_uart::Open(const char * const port, const int speed, const int parity) {
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if ( !Is_open() ) {
        printf("[BGC::BGC_uart] Open - failed to open at %s: %d\n", port, errno);
        return false;
    }
    if ( !Set_attributes(speed, parity) ) {
        Close();
        return false;
    }
    return true;
}

bool BGC_uart::Is_open() const {
    return fd >= 0;
}

void BGC_uart::Close() {
    close(fd);
}

int BGC_uart::Fd() {
    return fd;
}

BGC_uart::Poll_result BGC_uart::Poll(const int timeout_ms, short int event) {
    const int error_limit = 3;
    for ( int error_count = 0; error_count < error_limit; ++error_count ) {
        struct pollfd pfd; pfd.fd = fd; pfd.events = event;
        const int poll_ret = poll(&pfd, 1, timeout_ms);
        if ( poll_ret < 0 ) {
            printf("[BGC::BGC_uart] Poll - failed to poll: %d\n", errno);
        } else if ( poll_ret == 0 ) {
            return Poll_result::Timeout;
        } else {
            return Poll_result::Ready;
        }
    }
    printf("[BGC::BGC_uart] Poll - error limit reached\n");
    return Poll_result::Error;
}

bool BGC_uart::Discover_attributes(int & speed, int & parity, const int response_timeout_ms) {
    DOG_PRINT("[BGC::BGC_uart] Discover_attributes\n");
    // Spec: https://www.basecamelectronics.com/files/SimpleBGC_2_4_Serial_Protocol_Specification.pdf
    // BGC fw v2.40 only supports EVEN parity, v2.41 supports EVEN and NONE.
    const int test_parities[] = { PARITY_NONE, PARITY_EVEN /*PARITY_NONE, PARITY_EVEN, PARITY_ODD*/ };
    // The spec says default baud is 115200, but recommends testing others (does not specify which ones) too.
    const int test_bauds[] = { B115200, B57600 /*B460800, B256000, B115200, B57600*/ };
    for ( unsigned parity_i = 0; parity_i < sizeof(test_parities)/sizeof(test_parities[0]); ++parity_i ) {
        for ( unsigned baud_i = 0; baud_i < sizeof(test_bauds)/sizeof(test_bauds[0]); ++baud_i ) {
            speed = test_bauds[baud_i];
            parity = test_parities[parity_i];
            DOG_PRINT("[BGC::BGC_uart] trying attributes: speed=%d parity=%d\n", speed, parity);
            if ( Set_attributes(speed, parity) ) {
                const int get_board_info_tries = 3;
                for ( int try_nr = 0; try_nr < get_board_info_tries; ++try_nr ) {
                    if ( Get_board_info(response_timeout_ms) ) {
                        DOG_PRINT("[BGC::BGC_uart] discovered attributes: speed=%d parity=%d\n", speed, parity);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool BGC_uart::Send(const BGC_uart_msg & msg, const int zero_data_timeout) {
    const uint8_t * const buf = msg.Buf();
    const size_t buf_len      = msg.Buf_len();
    size_t sent = 0;
    for ( ; ; ) {
        const ssize_t just_sent = write(fd, &buf[sent], buf_len-sent);
        if ( just_sent >= 0 ) {
            sent += size_t(just_sent);
            if ( sent >= buf_len ) {
                return true;
            } else {
                const Poll_result poll_ret = Poll(zero_data_timeout, POLLOUT);
                if ( poll_ret == Poll_result::Timeout ) {
                    printf("[BGC::BGC_uart] Send - POLLOUT timeout\n");
                    break;
                } else if ( poll_ret == Poll_result::Error ) {
                    printf("[BGC::BGC_uart] Send - POLLOUT error\n");
                    break;
                }
            }
        } else {
            printf("[BGC::BGC_uart] Send - failed to write: %d\n", errno);
            break;
        }
    }
    return false;
}

bool BGC_uart::Recv(BGC_uart_msg & msg, const int zero_data_timeout) {
    while ( !msg.Is_fully_present() ) {
        const Poll_result poll_ret = Poll(zero_data_timeout, POLLIN);
        if ( poll_ret == Poll_result::Error ) return false;
        if ( poll_ret == Poll_result::Timeout ) break;
        if ( !Recv_partial(msg) ) return false;
    }
    return true;
}

bool BGC_uart::Recv_partial(BGC_uart_msg & msg) {
    if ( !msg.Is_first_byte_present() ) {
        if ( !Recv_partial_first_byte(msg) ) return false;
    }
    if ( msg.Is_first_byte_present() && !msg.Is_header_present() ) {
        if ( !Recv_partial_header(msg) ) return false;
    }
    if ( msg.Is_header_present() && !msg.Is_fully_present() ) {
        if ( !Recv_partial_body(msg) ) return false;
    }
    return true;
}

bool BGC_uart::Read_junk(const int zero_data_timeout_ms) {
    uint8_t junk_buf[256];
    Poll_result poll_result = Poll_result::Ready;
    while ( poll_result != Poll_result::Timeout ) {
        if ( read(fd, junk_buf, sizeof(junk_buf)) < 0 && errno != EAGAIN ) {
            printf("[BGC::BGC_uart] Read_junk - failed to read: %d\n", errno);
            return false;
        }
        poll_result = Poll(zero_data_timeout_ms, POLLIN);
        if ( poll_result == Poll_result::Error ) {
            printf("[BGC::BGC_uart] Read_junk - poll error\n");
            return false;
        }
    }
    return true;
}

//void BGC_uart::Read_messages_loop(const int timeout_ms = -1) {
//    BGC_uart_msg msg;
//    for ( ; ; ) {
//        if ( Recv(msg, timeout_ms) ) {
//            printf("[BGC::BGC_uart] read msg\n");
//            msg.Dump();
//        }
//    }
//}

BGC_uart::~BGC_uart() {
    if ( Is_open() ) {
        Close();
    }
}

bool BGC_uart::Set_attributes(const int speed, const int parity) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if ( tcgetattr(fd, &tty) != 0 ) {
        printf("[BGC::BGC_uart] Set_attributes - failed to tcgetattr: %d\n", errno);
        return false;
    }
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag |= parity;
    tty.c_cflag |= CREAD;
    tty.c_cflag |= CLOCAL;
    const int speed_res = cfsetspeed(&tty, speed);
    if ( speed_res != 0 ) {
        printf("[BGC::BGC_uart] Set_attributes - failed to cfsetspeed: %d %d\n", speed_res, speed);
        return false;
    }
    if ( tcsetattr(fd, TCSANOW, &tty) != 0 ) {
        printf("[BGC::BGC_uart] Set_attributes - failed to tcsetattr: %d\n", errno);
        return false;
    }
    return true;
}

bool BGC_uart::Get_board_info(const int response_timeout_ms) {
    DOG_PRINT("[BGC::BGC_uart] Get_board_info\n");
    BGC_uart_msg msg;
    msg.Build_OUT_CMD_BOARD_INFO();
    if ( !Read_junk() || !Send(msg) ) return false;
    msg.Reset();
    if ( Recv(msg, response_timeout_ms) && msg.Is_fully_valid() ) {
        if ( msg.Command_id() == SBGC_CMD_BOARD_INFO ) {
            #ifdef ENABLE_DOG_DEBUG
                DOG_PRINT("[BGC::BGC_uart] Get_board_info - SBGC_CMD_BOARD_INFO\n");
                const uint8_t  board_ver    = msg.Get_IN_CMD_BOARD_INFO_Board_ver();
                const uint16_t firmware_ver = msg.Get_IN_CMD_BOARD_INFO_Firmware_ver();
                const uint8_t  debug_mode   = msg.Get_IN_CMD_BOARD_INFO_Debug_mode();
                const uint16_t board_feat   = msg.Get_IN_CMD_BOARD_INFO_Board_features();
                const uint8_t  conn_flags   = msg.Get_IN_CMD_BOARD_INFO_Connection_flags();
                DOG_PRINT("[BGC::BGC_uart] board ver        = %u.%u\n",    board_ver/10, board_ver%10);
                DOG_PRINT("[BGC::BGC_uart] firmware ver     = %u.%ub%u\n", firmware_ver/1000
                                                                      , firmware_ver/10%100
                                                                      , firmware_ver%10);
                DOG_PRINT("[BGC::BGC_uart] debug mode       = %u\n",       debug_mode);
                DOG_PRINT("[BGC::BGC_uart] board features   = %04x\n",     board_feat);
                DOG_PRINT("[BGC::BGC_uart] connection flags = %02x\n",     conn_flags);
            #endif
            return true;
        } else {
            printf("[BGC::BGC_uart] Get_board_info - unexpected response\n");
            msg.Dump();
        }
    }
    return false;
}

bool BGC_uart::Recv_partial_first_byte(BGC_uart_msg & msg) {
    if ( !Recv_partial_bytes(msg, 1) ) return false;
    if ( msg.Is_first_byte_present() && !msg.Is_first_byte_valid() ) msg.Reset();
    return true;
}

bool BGC_uart::Recv_partial_header(BGC_uart_msg & msg) {
    if ( !Recv_partial_bytes(msg, BGC_uart_msg::Header_bytes) ) return false;
    if ( msg.Is_header_present() && !msg.Is_header_valid() ) msg.Reset();
    return true;
}

bool BGC_uart::Recv_partial_body(BGC_uart_msg & msg) {
    if ( !Recv_partial_bytes(msg, msg.Buf_len()) ) return false;
    if ( msg.Is_fully_present() && !msg.Is_fully_valid() ) msg.Reset();
    return true;
}

bool BGC_uart::Recv_partial_bytes(BGC_uart_msg & msg, const uint8_t need_bytes) {
    const uint8_t bytes_present = msg.Bytes_present();
    uint8_t * const buf = msg.Buf();
    const ssize_t just_read = read(fd, &buf[bytes_present], need_bytes - bytes_present);
    if ( just_read > 0 ) {
        msg.Add_bytes(just_read);
    } else if ( just_read < 0 && errno != EAGAIN ) {
        printf("[BGC::BGC_uart] Recv_partial_bytes - failed to read: %d\n", errno);
        return false;
    }
    return true;
}

} // namespace BGC

