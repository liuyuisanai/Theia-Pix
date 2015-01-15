#pragma once

#include "unique_file.hpp"

class Tone
{
public:
	Tone();
	void key_press() const;
	void mode_switch() const;
	void arm_failed() const;
	void timeout() const;
private:
	void play(int tone) const;
	unique_file fd;
};
