extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>

#include <board_config.h>
#include <display.h>

#include <ctype.h>
#include <cstdio>
#include <cstring>


namespace {

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

static void
usage(const char name[])
{
	fprintf(stderr,
		"Usage: %s on/off\n",
		"          test-blink/test-nop\n",
		"          cmd hex [data] hex ...\n",
		name);
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
		return 1;
	}

	if (streq(argv[1], "on"))
	{
		up_display_mcu_setup();
		display_init();
	}
	else if (streq(argv[1], "off"))
	{
		display_shutdown();
	}
	else if (streq(argv[1], "test-nop"))
	{
		up_display_mcu_setup();
		display_turn_on();
		for (int i = 0; i < 1024*1024; ++i)
			display_test_nop(4);
		//for (int i = 0; i < 64; ++i)
		//	display_test_nop(16384);
		display_turn_off();
	}
	else if (streq(argv[1], "test-blink"))
	{
		up_display_mcu_setup();
		display_init();

		auto c = (volatile char *) LCD_SERVICE_AREA;

		*c = 0xa5;

		printf("Press enter.\n");
		getchar();

		*c = 0xa4;

		display_shutdown();
	}
	else if (streq(argv[1], "cmd") or streq(argv[1], "data"))
	{
		auto p = (volatile uint8_t *) LCD_SERVICE_AREA;
        stm32_gpiowrite(GPIO_LCD_A0, 0);

		for (int i = 1; i < argc; ++i)
		{
			if (streq(argv[i], "cmd"))
                stm32_gpiowrite(GPIO_LCD_A0, 0);
			else if (streq(argv[i], "data"))
                stm32_gpiowrite(GPIO_LCD_A0, 1);
			else
			{
				if (isxdigit(argv[i][0]) and isxdigit(argv[i][1]) and !argv[i][2])
				{
					unsigned x0 = toupper(argv[i][0]);
					unsigned d0 = isdigit(x0) ? (x0 - '0') : ((x0) - 'A' + 10);
					unsigned x1 = toupper(argv[i][1]);
					unsigned d1 = isdigit(x1) ? (x1 - '0') : ((x1) - 'A' + 10);
					uint8_t x = (d0 << 4) | d1;
					*p = x;
					//fprintf(stderr, "%i %02x\n", cmd, x);
				}
				else
				{
					fprintf(stderr,
						"Invalid value %s\n",
						argv[i]);
				}
			}
		}
	}
	else
	{
		usage(argv[0]);
		return 1;
	}

	return 0;
}
