#include <nuttx/config.h>
#include <fcntl.h>

#include <drivers/drv_tone_alarm.h>

#include "tones.hpp"

Tone::Tone() : fd(open(TONEALARM_DEVICE_PATH, O_WRONLY)) {}

void
Tone::play(int tone)
{
	if (fd.get() != -1)
		ioctl(fd.get(), TONE_SET_ALARM, tone);
}

void
Tone::key_press() { play(TONE_NOTIFY_POSITIVE_TUNE); }
