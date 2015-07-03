#include "block.hpp"

#include <display.h>

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

MultiBlock::~MultiBlock()
{
    for (BlockVector::iterator it = blocks.begin(); it < blocks.end(); it++)
    {
        delete *it;
    }
}

void MultiBlock::add(Block *block)
{
    block->parent = this;
    blocks.push_back(block);
}

void MultiBlock::draw()
{
    for (BlockVector::iterator it = blocks.begin(); it < blocks.end(); it++)
    {
        (*it)->draw();
    }
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

TextBlock::TextBlock(const std::string &pText, int pX, int pY, int pDigitImageId, int pLetterImageId) :
    Block(pX, pY, 0, 0), text(pText), digitImageId(pDigitImageId), letterImageId(pLetterImageId)
{
}

void TextBlock::draw()
{
    const int letterCount = ('z' - 'a') + 1;
    int px = getAbsoluteX();
    int py = getAbsoluteY();
    for (size_t i = 0; i < text.size(); i++)
    {
        int c = text[i];
        if (c >= 'a' && c <= 'z')
        {
            c -= 'a';
            c = letterImageId + c;
        }
        else if (c >= 'A' && c <= 'Z')
        {
            c -= 'A';
            c = letterImageId + c + letterCount;
        }
        else if (c >= '0' && c <= '9')
        {
            c -= '0';
            c = digitImageId + c;
        }

        const ImageInfo &info = imageInfo[c];
        display_bitmap(px, py, info.w, info.h, imageData + info.offset);
        px += info.w;
    }
}

