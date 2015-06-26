#include <nuttx/config.h>

//extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <drivers/drv_frame_button.h>
#include <systemlib/systemlib.h>

#include "button_state.hpp"

press_type
handle_kbd_state(KbdButtonState & btn, hrt_abstime now)
{

	unsigned long_press_min = LONG_KEYPRESS_DURATION_us;
	unsigned long_press_max = LONG_KEYPRESS_DURATION_us + FRAME_BUTT_SCAN_INTERVAL_usec;

	if (btn.time_released)
	{
		unsigned dt = btn.time_released - btn.time_pressed;
		if (dt < long_press_min) {
            DOG_PRINT("short press\n");
            btn.time_released = 0;
            btn.time_pressed = 0;
            return SHORT_KEYPRESS;
        }
		else if (dt < long_press_max) {
            DOG_PRINT("LONG press\n");
            btn.time_released = 0;
            btn.time_pressed = 0;
            return LONG_KEYPRESS;
        }
	}
	else
	{
		unsigned dt = now - btn.time_pressed;
		if (long_press_min <= dt)
		{
			if (dt < long_press_max)
			{
                DOG_PRINT("LONG press from else\n");
                return LONG_KEYPRESS;
			}
            else {
                DOG_PRINT("Nothing pressed\n");
                return NOT_PRESSED;
                // Do nothing if user holds it too long
            }
		}
	}
    return NOT_PRESSED;
}
