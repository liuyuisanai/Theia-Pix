#pragma once

#include "unique_file.hpp"

class Tone
{
public:
	Tone();
	void key_press();
private:
	void play(int tone);
	unique_file fd;
};
