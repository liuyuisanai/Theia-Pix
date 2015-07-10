#include "font.hpp"

#include <string.h>

#include "images/images.h"

Font Font::LucideGrandeBig(IMAGE_SCREENS_FONTS_LUCIDAGRANDE_30_SMALL_A);
Font Font::LucideGrandeMed(IMAGE_SCREENS_FONTS_LUCIDAGRANDE_22_SMALL_A);
Font Font::LucideGrandeSmall(IMAGE_SCREENS_FONTS_LUCIDAGRANDE_15_SMALL_A);
Font Font::LucideGrandeTiny(IMAGE_SCREENS_FONTS_LUCIDAGRANDE_12_SMALL_A);

Font::Font(int pFirstSymbolImageId)
{
    firstSymbolImageId = pFirstSymbolImageId;
}

int Font::getSymbolImageId(char c) const
{
    int id = -1;
    int letterCount = 'z' - 'a' + 1;
    int symbolOffset = letterCount + letterCount + 10;

    if (c >= 'a' && c <= 'z')
    {
        c -= 'a';
        id = firstSymbolImageId + c;
    }
    else if (c >= 'A' && c <= 'Z')
    {
        c -= 'A';
        id = firstSymbolImageId + letterCount + c;
    }
    else if (c >= '0' && c <= '9')
    {
        c -= '0';
        id = firstSymbolImageId + letterCount + letterCount + c;
    }
    else if (c == '%')
    {
        id = firstSymbolImageId + symbolOffset;
    }
    else if (c == ' ')
    {
        id = firstSymbolImageId + symbolOffset + 1;
    }
    else if (c == '.')
    {
        id = firstSymbolImageId + symbolOffset + 2;
    }

    return id;
}

size_t Font::getTextWidth(const char *text)
{
    size_t width = 0;

    for (size_t i = 0; i < strlen(text); i++)
    {
        char c = text[i];

        int imageId = getSymbolImageId(c);

        if (imageId == -1)
        {
            printf("Font: Symbol '%c' (0x%x) was not found\n", c, c);
            continue;
        }

        ImageInfo info = imageInfo[imageId];
        width += info.w;
    }

    return width;
}

size_t Font::getTextHeight(const char *text)
{
    size_t height = 0;

    for (size_t i = 0; i < strlen(text); i++)
    {
        char c = text[i];

        int imageId = getSymbolImageId(c);

        if (imageId == -1)
        {
            printf("Font: Symbol '%c' (0x%x) was not found\n", c, c);
            continue;
        }

        ImageInfo info = imageInfo[imageId];
        if (info.h > height)
        {
            height = info.h;
        }
    }

    return height;
}
