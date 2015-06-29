#include <nuttx/config.h>

//extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <drivers/drv_frame_button.h>
#include <systemlib/systemlib.h>

#include "button_state.hpp"

namespace frame_kbd{
press_type
handle_kbd_state(KbdButtonState & btn,KbdButtonState & last_btn, hrt_abstime now, press_type last_press)
{

	unsigned multiclick_space = MULTICLICK_SPACING_us;   
	unsigned long_press_min = LONG_KEYPRESS_DURATION_us;
	unsigned long_press_max = LONG_KEYPRESS_DURATION_us + FRAME_BUTT_SCAN_INTERVAL_usec;

	if (btn.time_released)
	{
		unsigned dt = btn.time_released - btn.time_pressed;
        unsigned release_dt = now - btn.time_released;
		if (dt < long_press_min && release_dt > multiclick_space) {
            if (last_press == SUSPECT_FOR_DOUBLE) {
                btn.time_released = 0;
                btn.time_pressed = 0;
                return SHORT_KEYPRESS;
            }
            else if (last_press == SUSPECT_FOR_TRIPLE) {
                btn.time_released = 0;
                btn.time_pressed = 0;
                return DOUBLE_CLICK;
            }
            else {
                btn.time_released = 0;
                btn.time_pressed = 0;
                return SHORT_KEYPRESS;
            }
        }
        // Multicklick support
		else if (dt < long_press_min && release_dt < multiclick_space) {
            if (last_press == SUSPECT_FOR_DOUBLE) {
                if (btn.time_released == last_btn.time_released) {
                    return SUSPECT_FOR_DOUBLE;
                }
                last_btn.time_released = btn.time_released;
                return SUSPECT_FOR_TRIPLE;
            }
            else if (last_press == SUSPECT_FOR_TRIPLE) {
                if (btn.time_released == last_btn.time_released) {
                    return SUSPECT_FOR_TRIPLE;
                }
                return TRIPLE_CLICK;
            }
            else if (last_press == TRIPLE_CLICK) {
                btn.time_released = 0;
                btn.time_pressed = 0;
                return NOT_PRESSED;
            }
            else {
                last_btn.time_released = btn.time_released;
                return SUSPECT_FOR_DOUBLE;
            }
        }
	}
	else
	{
		unsigned dt = now - btn.time_pressed;
		if (long_press_min <= dt)
		{
			if (dt < long_press_max)
			{
                return LONG_KEYPRESS;
			}
            else {
                // Do nothing if user holds it too long
                return NOT_PRESSED;
            }
		}
	}
    return last_press;
}
}
