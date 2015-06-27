#include <drivers/drv_hrt.h>

#include "kbd_reader.hpp"
#include "settings.hpp"


using KbdButtonState = ButtonState<
	pressed_mask_t, FRAME_BUTT_SCAN_BUFFER_N_ITEMS,
	hrt_abstime, FRAME_BUTT_SCAN_INTERVAL_usec
>;

typedef enum{
    NOT_PRESSED = 0,
    LONG_KEYPRESS,
    SHORT_KEYPRESS,
    SUSPECT_FOR_DOUBLE,
    DOUBLE_CLICK,
    SUSPECT_FOR_TRIPPLE,
    TRIPLE_CLICK,
}press_type;

press_type handle_kbd_state(KbdButtonState & btn, KbdButtonState & last_btn, hrt_abstime now, press_type last_press);
