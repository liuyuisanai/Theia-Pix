#include <nuttx/config.h>
#include <fcntl.h>
#include <stdio.h>

#include <drivers/drv_tone_alarm.h>

#include "tones.hpp"

Tone::Tone() : fd(open(TONEALARM_DEVICE_PATH, O_WRONLY)) {}

void
Tone::play(int tone) const
{
	if (fd.get() != -1)
		ioctl(fd.get(), TONE_SET_ALARM, tone);
}

void
Tone::mode_switch() const
{
	fprintf(stderr, "TONE Mode Switch.\n");
	play(TONE_NOTIFY_POSITIVE_TUNE);
}

void
Tone::key_press() const
{
	fprintf(stderr, "TONE Key Press.\n");
	play(TONE_NOTIFY_POSITIVE_TUNE);
}

void
Tone::arm_failed() const
{
	fprintf(stderr, "TONE Arm Failed.\n");
	play(TONE_NOTIFY_NEGATIVE_TUNE);
}

void
Tone::timeout() const
{
	fprintf(stderr, "TONE Arm Failed.\n");
	play(TONE_NOTIFY_NEGATIVE_TUNE);
}
