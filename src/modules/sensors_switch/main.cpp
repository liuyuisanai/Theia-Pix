extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <nuttx/config.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <board_config.h>
#include <drivers/drv_adc.h>
#include <uORB/uORB.h>
#include <uORB/topics/system_power.h>

#define SENSOR_SWITCH_GPIO GPIO_VDD_3V3_SENSORS_EN

#ifndef N_ADC_CHANNELS
// same as in sensors.cpp
# define N_ADC_CHANNELS 12
#endif

namespace {

inline bool
streq(const char *a, const char *b) {
	return not std::strcmp(a, b);
}

void
report_system_voltage()
{
	int sub = orb_subscribe(ORB_ID(system_power));
	system_power_s system_power;
	orb_copy(ORB_ID(system_power), sub, &system_power);
	orb_unsubscribe(sub);
	printf("System voltage: %.2f\n", (double)system_power.voltage5V_v);
	printf("Servo valid: %d, brick valid: %d, usb conncted: %d\n", system_power.servo_valid,
		system_power.brick_valid, system_power.usb_connected);
}

void
report_sensor_voltage()
{
	int fd_adc = open(ADC_DEVICE_PATH, O_RDONLY);

	if (fd_adc < 0) {
		fprintf(stderr, "Failed to open" ADC_DEVICE_PATH "\n");
		return;
	}

	struct adc_msg_s adc[N_ADC_CHANNELS];
	int ret, i;
	ret = read(fd_adc, &adc, sizeof(adc));
	if (ret < 0) {
		perror(ADC_DEVICE_PATH " read");
		fprintf(stderr, ADC_DEVICE_PATH " read failed.");
	}
	else
	{
		const int n = ret / sizeof(*adc);
		bool ok = false;
		for (i = 0; i < n; ++i) {
			fprintf(stderr, "%i adc %i data %4u (0x%04x)\n",
					i, adc[i].am_channel,
					adc[i].am_data, adc[i].am_data);
#ifdef ADC_SENSORS_VOLTAGE_CHANNEL
			if (adc[i].am_channel == ADC_SENSORS_VOLTAGE_CHANNEL)
			{
				printf("Sensor voltage: %u %.2f\n"
					, adc[i].am_data
					, adc[i].am_data * ADC_SENSORS_VOLTAGE_SCALE
				);
				ok = true;
			}
#endif
		}
		if (not ok)
			fprintf(stderr, "Sensor voltage undetermined.\n");
	}
	close(fd_adc);
}

} // end of namespace

int
main(int argc, const char * const * const argv)
{
	if (argc < 2)
	{
		fprintf(stderr,
			"Usage: %s command [...]\n"
			"\n"
			"Command is one of: on, off, status, 10ms."
			" Each could be repeated multiple times\n"
			, argv[0]);
	}

	for (int i = 1; i < argc; ++i)
	{
		fprintf(stderr, "%s\n", argv[i]);
		if (streq(argv[i], "on"))
			stm32_gpiowrite(SENSOR_SWITCH_GPIO, 1);
		else if (streq(argv[i], "off"))
			stm32_gpiowrite(SENSOR_SWITCH_GPIO, 0);
		else if (streq(argv[i], "status"))
		{
			bool on = stm32_gpioread(SENSOR_SWITCH_GPIO);
			printf("Sensor power %s\n", on ? "on" : "off");
			report_sensor_voltage();
			report_system_voltage();
		}
		else if (streq(argv[i], "1ms"))
			usleep(10000);
		else { fprintf(stderr, "Unknown command: \"%s\"\n", argv[i]); }
	}

	return 0;
}
