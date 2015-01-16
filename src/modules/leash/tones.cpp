#include <nuttx/config.h>
#include <fcntl.h>
#include <stdio.h>

#include <drivers/drv_tone_alarm.h>

#include "debug.hpp"
#include "tones.hpp"


Tone::Tone() : fd(open(TONEALARM_DEVICE_PATH, O_WRONLY)) {}

void
Tone::play(int tone) const
{
	if (fd.get() != -1)
		ioctl(fd.get(), TONE_SET_ALARM, tone);
}

void
Tone::arm_failed() const
{
	say("TONE Arm Failed.");
	play(TONE_NOTIFY_NEGATIVE_TUNE);
}

void
Tone::key_press() const
{
	say("TONE Key Press.");
	play(TONE_NOTIFY_POSITIVE_TUNE);
}

void
Tone::key_press_timeout() const
{
	say("TONE key press Timeout.");
	play(TONE_NOTIFY_NEGATIVE_TUNE);
}

void
Tone::mode_switch() const
{
	say("TONE Mode Switch.");
	play(TONE_NOTIFY_POSITIVE_TUNE);
}
