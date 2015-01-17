#pragma once

namespace indication { namespace leds {

void set_default();
void set_pattern_once(unsigned led, uint32_t pattern);
void set_pattern_repeat(unsigned led, uint32_t pattern);

void update();
void status();

}} // end of namespace indication::leds
