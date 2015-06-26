#include <nuttx/config.h>

extern "C" __EXPORT int main(int argc, const char *argv[]);

#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <drivers/drv_frame_button.h>
#include <drivers/drv_hrt.h>
#include <systemlib/systemlib.h>

#include "kbd_reader.hpp"
#include "settings.hpp"

namespace {

static bool daemon_should_run = false;
static bool daemon_running = false;

typedef enum{
    NOT_PRESSED = 0,
    LONG_KEYPRESS,
    SHORT_KEYPRESS
}press_type;

using KbdButtonState = ButtonState<
	pressed_mask_t, FRAME_BUTT_SCAN_BUFFER_N_ITEMS,
	hrt_abstime, FRAME_BUTT_SCAN_INTERVAL_usec
>;

void
update_buttons(KbdButtonState & s, hrt_abstime now, int f_kbd)
{
	pressed_mask_t masks[FRAME_BUTT_SCAN_BUFFER_N_ITEMS];
	// f_kbd should always be full enough.  Ignore read result.
	read(f_kbd, masks, sizeof(masks));
	s.update(now, masks);
}

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

static int
daemon(int argc, char *argv[])
{
	int f_kbd_masks = open(FRAME_BUTT_DEVICE_PATH, O_RDONLY);
	if (f_kbd_masks == -1) {
		perror("frame open(" FRAME_BUTT_DEVICE_PATH ")");
		return 1;
	}

	KbdButtonState btn;

	daemon_running = true;
	fprintf(stderr, "%s has started.\n", argv[0]);

	while (daemon_should_run)
	{
		usleep(FRAME_BUTT_SCAN_INTERVAL_usec);
		hrt_abstime now = hrt_absolute_time();
		update_buttons(btn, now, f_kbd_masks);

		/*
		 * Button events have priority to timeouts as
		 * while button is pressed no timeout should happen, and
		 * then button, most probably, will reset the timeout itself.
		 */
        press_type event = NOT_PRESSED;
        event = handle_kbd_state(btn, now);
        if (event != NOT_PRESSED){ 
            DOG_PRINT("Frame button event: %d ", event);
            //fprintf(stderr, "Frame button event: %d\n");
            DOG_PRINT(event == SHORT_KEYPRESS ? "SHORT_KEYPRESS\n" : "LONG_KEYPRESS\n");
        }
	}

	close(f_kbd_masks);
	daemon_running = false;

	fprintf(stderr, "%s has stopped.\n", argv[0]);
	return 0;
}

static inline bool
streq(const char a[], const char b[]) { return std::strcmp(a, b) == 0; }

static void
usage(const char name[])
{ std::fprintf(stderr, "Usage: %s start|stop|status\n\n", name); }

} // end of anonymous namespace

int
main(int argc, const char *argv[])
{
	if (argc != 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "start"))
	{
		if (daemon_running)
		{
			fprintf(stderr, "%s is already running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = true;
		task_spawn_cmd(argv[0],
				SCHED_DEFAULT,
				SCHED_PRIORITY_DEFAULT,
				CONFIG_TASK_SPAWN_DEFAULT_STACKSIZE,
				daemon,
				argv);
	}
	else if (streq(argv[1], "status"))
	{
		if (daemon_running) { printf("%s is running.\n", argv[0]); }
		else { printf("%s is NOT running.\n", argv[0]); }
	}
	else if (streq(argv[1], "stop"))
	{
		if (not daemon_running)
		{
			fprintf(stderr, "%s is NOT running.\n", argv[0]);
			return 1;
		}
		daemon_should_run = false;
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	fprintf(stderr, "main() is returning 0");
	return 0;
}
