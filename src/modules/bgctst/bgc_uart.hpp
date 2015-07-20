#ifndef __BGCTST_BGC_UART_HPP_INCLUDED__
#define __BGCTST_BGC_UART_HPP_INCLUDED__

#include <termios.h>

namespace BGC {

class BGC_uart_msg;

class BGC_uart {
public:
    enum class Poll_result { Ready, Timeout, Error };
    
    enum { PARITY_NONE = 0, PARITY_EVEN = PARENB, PARITY_ODD = PARENB | PARODD };
    
    enum { DEFAULT_SPEED  = B115200     };
    enum { DEFAULT_PARITY = PARITY_NONE };
    
    enum { DEFAULT_REQUEST_TIMEOUT_MS    = 50   };
    enum { DEFAULT_RESPONSE_TIMEOUT_MS   = 500  };
    enum { DEFAULT_READ_JUNK_TIMEOUT     = 10   };
    
public:
    // Initializes this object, leaves uart connection closed, does not perform any IO.
    BGC_uart();
    
    BGC_uart(const char * const port, const int speed = DEFAULT_SPEED, const int parity = DEFAULT_PARITY);
    
    bool Open(const char * const port = "/dev/ttyS5", const int speed = DEFAULT_SPEED, const int parity = DEFAULT_PARITY);
    bool Is_open() const;
    void Close();
    int Fd();
    
    // Can be used when you just need to poll this uart connection for just one of POLLIN / POLLOUT.
    // Returns values:
    // Poll_result::Ready   - uart is ready to perform action specified by events (POLLIN | POLLOUT)
    // Poll_result::Timeout - timeout has elapsed and uart is not ready to perform specified action
    // Poll_result::Error   - error occurred while polling
    Poll_result Poll(const int timeout_ms, short int event);
    
    // Discovers uart attributes - speed/parity by sending request for board info until a valid response is received.
    // On success, returns true and sets speed/parity variables and leaves current uart connection set to those variables.
    // On failure, returns false and leaves speed/parity variables and uart connection open, but with undefined speed/parity.
    bool Discover_attributes(int & speed, int & parity, const int response_timeout_ms = DEFAULT_RESPONSE_TIMEOUT_MS);
    
    // Sends the given message over current uart connection, fails if no bytes transmitted in specified timeout.
    // Returns true on success, false if message was not fully delivered.
    bool Send(const BGC_uart_msg & msg, const int zero_data_timeout = DEFAULT_REQUEST_TIMEOUT_MS);
    
    // Receives one full message (it can be already partially received beforehand).
    // Returns true if message received or no bytes received in specified timeout. (Query message to see if it has arrived.)
    // Returns false only if an error (during polling or reading) occurred.
    bool Recv(BGC_uart_msg & msg, const int zero_data_timeout = DEFAULT_RESPONSE_TIMEOUT_MS);
    
    // Expects a message that is not Is_fully_present yet. (Reset a message to read a new one into the same struct.)
    // Receives whatever bytes it can without waiting, returns true.
    // Returns false only on error during read.
    bool Recv_partial(BGC_uart_msg & msg);
    
    // Reads and ignores any data on uart connection. Returns when no data has arrived for the specified timeout.
    // Returns true if timeout has elapsed, false if an error occurred during read / poll.
    bool Read_junk(const int zero_data_timeout_ms = DEFAULT_READ_JUNK_TIMEOUT);
    
    // For debugging purposes, not used in production code
    // void Read_messages_loop(const int timeout_ms = -1);
    
    ~BGC_uart();
    
private:
    // Sets attributes of current uart connection to specified speed and parity. (Does not test if communication is successful.)
    // Returns true on success, false if failed to set the attributes for any reason.
    bool Set_attributes(const int speed, const int parity);
    
    // Sends request for board info on current uart connection, returns true if receives valid response, false otherwise.
    bool Get_board_info(const int response_timeout_ms = DEFAULT_RESPONSE_TIMEOUT_MS);
    
    bool Recv_partial_first_byte(BGC_uart_msg & msg);
    bool Recv_partial_header(BGC_uart_msg & msg);
    bool Recv_partial_body(BGC_uart_msg & msg);
    bool Recv_partial_bytes(BGC_uart_msg & msg, const uint8_t need_bytes);
    
private:
    int fd;
    
private:
    BGC_uart(const BGC_uart &);
    BGC_uart & operator=(const BGC_uart &);
};

}

#endif
