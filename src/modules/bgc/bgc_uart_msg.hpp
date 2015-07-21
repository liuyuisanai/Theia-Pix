#ifndef __BGCTST_BGC_UART_MSG_HPP_INCLUDED__
#define __BGCTST_BGC_UART_MSG_HPP_INCLUDED__

#include <stdint.h>

#define SBGC_CMD_MAX_BYTES 255

#define SBGC_CMD_CONFIRM            67
#define SBGC_CMD_EXECUTE_MENU       69
#define SBGC_CMD_MOTORS_ON          77
#define SBGC_CMD_BOARD_INFO         86
#define SBGC_CMD_MOTORS_OFF         109
#define SBGC_CMD_RESET              114
#define SBGC_CMD_ERROR              255

#define SBGC_MENU_CMD_CALIB_ACC    6
#define SBGC_MENU_CMD_CALIB_GYRO   9
#define SBGC_MENU_CMD_MOTOR_TOGGLE 10

namespace BGC {
    class BGC_uart_msg {
    public:
        enum {
              Header_bytes   = 4
            , Tail_bytes     = 1
            , Non_data_bytes = Header_bytes + Tail_bytes
            , Max_buf_size   = SBGC_CMD_MAX_BYTES
            , Max_data_size  = Max_buf_size - Non_data_bytes
        };
        
    public:
        BGC_uart_msg() : bytes_present(0) { }
        
        uint8_t Bytes_present() const     { return bytes_present; }
        void Add_bytes(const uint8_t num) { bytes_present += num; }
        
        const uint8_t * Buf()     const { return &angle_bracket;                      }
        uint8_t       * Buf()           { return &angle_bracket;                      }
        uint8_t         Buf_len() const { return data_size + uint8_t(Non_data_bytes); } // Only makes sense if Is_header_present
        
        uint8_t         Command_id() const { return command_id; }
        uint8_t         Data_size()  const { return data_size;  }
        const uint8_t * Data()       const { return body;       }
        uint8_t *       Data()             { return body;       }
        
        void Reset() {
            bytes_present = 0;
        }
        
        bool Is_first_byte_present() const {
            return bytes_present >= 1;
        }
        
        bool Is_header_present() const {
            return bytes_present >= Header_bytes;
        }
        
        bool Is_fully_present() const {
            return Is_header_present() && bytes_present >= Buf_len();
        }
        
        bool Is_first_byte_valid() const {
            return Is_first_byte_present() && angle_bracket == uint8_t('>');
        }
        
        bool Is_header_valid() const {
            return Is_header_present()
                && angle_bracket   == uint8_t('>')
                && data_size       <= Max_data_size
                && header_checksum == Calc_header_checksum();
        }
        
        bool Is_fully_valid() const {
            return Is_header_valid()
                && Is_fully_present()
                && body[data_size] == Calc_body_checksum();
        }
        
        void Build_OUT_CMD_BOARD_INFO() {
            angle_bracket   = uint8_t('>');
            command_id      = uint8_t(SBGC_CMD_BOARD_INFO);
            data_size       = uint8_t(0);
            header_checksum = Calc_header_checksum();
            body[data_size] = Calc_body_checksum();
            bytes_present   = 5;
        }
        
        void Build_OUT_CMD_MOTORS_ON() {
            angle_bracket   = uint8_t('>');
            command_id      = uint8_t(SBGC_CMD_MOTORS_ON);
            data_size       = uint8_t(0);
            header_checksum = Calc_header_checksum();
            body[data_size] = Calc_body_checksum();
            bytes_present   = 5;
        }
        
        void Build_OUT_CMD_MOTORS_OFF() {
            angle_bracket   = uint8_t('>');
            command_id      = uint8_t(SBGC_CMD_MOTORS_OFF);
            data_size       = uint8_t(0);
            header_checksum = Calc_header_checksum();
            body[data_size] = Calc_body_checksum();
            bytes_present   = 5;
        }
        
        void Build_OUT_CMD_EXECUTE_MENU(const uint8_t cmd_id) {
            angle_bracket   = uint8_t('>');
            command_id      = uint8_t(SBGC_CMD_EXECUTE_MENU);
            data_size       = uint8_t(1);
            header_checksum = Calc_header_checksum();
            body[0]         = cmd_id;
            body[data_size] = Calc_body_checksum();
            bytes_present   = 6;
        }
        
        uint8_t  Get_IN_CMD_BOARD_INFO_Board_ver()        const { return body[0];                                      }
        uint16_t Get_IN_CMD_BOARD_INFO_Firmware_ver()     const { return uint16_t(body[1]) + (uint16_t(body[2]) << 8); }
        uint8_t  Get_IN_CMD_BOARD_INFO_Debug_mode()       const { return body[3];                                      }
        uint16_t Get_IN_CMD_BOARD_INFO_Board_features()   const { return uint16_t(body[4]) + (uint16_t(body[5]) << 8); }
        uint8_t  Get_IN_CMD_BOARD_INFO_Connection_flags() const { return body[6];                                      }
        
        uint8_t         Get_IN_CMD_CONFIRM_Cmd()          const { return body[0];       }
        uint8_t         Get_IN_CMD_CONFIRM_Data_size()    const { return data_size - 1; }
        const uint8_t * Get_IN_CMD_CONFIRM_Data()         const { return &body[1];      }
        
        void Dump() const {
            const uint8_t * const buf = Buf();
            const uint8_t buf_len     = Buf_len();
            printf("[BGC::BGC_uart_msg] dump");
            if ( Is_fully_valid() ) { printf(" (valid)"); } else { printf(" (invalid)"); }
            for ( uint8_t i = 0; i < buf_len && i < SBGC_CMD_MAX_BYTES; ++i ) {
                printf(" %02x", unsigned(buf[i]));
            }
            printf("\n");
        }
        
    private:
        enum { Body_buf_size = Max_data_size + 1 };
        
    private:
        uint8_t Calc_header_checksum() const {
            return command_id + data_size; // Should also be mod 256, but uint8_t guarantees that.
        }
        
        uint8_t Calc_body_checksum() const {
            uint8_t sum = 0;
            for ( uint8_t i = 0; i < data_size; ++i ) {
                sum += body[i]; // Should also be mod 256, but uint8_t guarantees that.
            }
            return sum;
        }
        
    private:
        uint8_t bytes_present;
        uint8_t angle_bracket;
        uint8_t command_id;
        uint8_t data_size;
        uint8_t header_checksum;
        uint8_t body[Body_buf_size];
        
    private:
        BGC_uart_msg(const BGC_uart_msg &);
        BGC_uart_msg & operator=(const BGC_uart_msg &);
    };
}

#endif
