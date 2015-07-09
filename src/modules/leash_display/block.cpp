#include "block.hpp"

#include <stdio.h>

#include <display.h>
#include <string.h>

#include "images/images.h"

Block::Block(int pX, int pY, int pWidth, int pHeight) :
    x(pX), y(pY), width(pWidth), height(pHeight), parent(nullptr)
{

}

int Block::getAbsoluteX()
{
    int a = x;

    if (parent != nullptr)
    {
        a += parent->getAbsoluteX();
    }

    return a;
}

int Block::getAbsoluteY()
{
    int a = y;

    if (parent != nullptr)
    {
        a += parent->getAbsoluteY();
    }

    return a;
}

BitmapBlock::BitmapBlock(int pX, int pY, int pWidth, int pHeight, const unsigned char *pBitmap) :
    Block(pX, pY, pWidth, pHeight), bitmap(pBitmap)
{
}

BitmapBlock::BitmapBlock(int pX, int pY, int pImageId) :
    BitmapBlock(pX, pY, imageInfo[pImageId].w, imageInfo[pImageId].h,
                imageData + imageInfo[pImageId].offset)
{
}

void BitmapBlock::draw()
{
    display_bitmap(getAbsoluteX(), getAbsoluteY(), width, height, bitmap);
}

TextBlock::TextBlock(const char *pText, int pX, int pY, const Font *pFont) :
    Block(pX, pY, 0, 0), text(pText), font(pFont)
{
}

void TextBlock::draw()
{
    int px = getAbsoluteX();
    int py = getAbsoluteY();

    for (size_t i = 0; i < strlen(text); i++)
    {
        char c = text[i];

        int imageId = font->getSymbolImageId(c);

        if (imageId == -1)
        {
            printf("Symbol '%c' (0x%x) was not found\n", c, c);
            continue;
        }

        ImageInfo info = imageInfo[imageId];

        display_bitmap(px, py, info.w, info.h, imageData + info.offset);

        px += info.w;
    }
}
