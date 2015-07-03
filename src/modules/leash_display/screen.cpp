#include "screen.hpp"

#include <display.h>

#include "block.hpp"
#include "images/images.h"

void Screen::showLogo()
{
    BitmapBlock blockLogo(0, 0, IMAGE_SCREENS_LOGO);
    blockLogo.draw();
}
