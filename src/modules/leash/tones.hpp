#pragma once

#include "unique_file.hpp"

class Tone
{
public:
	Tone();
	void arm_failed() const;
	void key_press() const;
	void key_press_timeout() const;
	void mode_switch() const;
private:
	void play(int tone) const;
	unique_file fd;
};
