#pragma once

#include "font.hpp"

class Block {
public:
    int x;
    int y;
    int width;
    int height;
    Block *parent;

    Block(int pX, int pY, int pWidth, int pHeight);

    virtual void draw() = 0;
    virtual ~Block() {}

    int getAbsoluteX();
    int getAbsoluteY();
};

class BitmapBlock : public Block {
public:
    BitmapBlock(int pX, int pY, int pImageId);
    BitmapBlock(int pX, int pY, int pWidth, int pHeight, const unsigned char *pBitmap);
    void draw();
protected:
    const unsigned char *bitmap;
};

class TextBlock : public Block {
public:
    TextBlock(const char *pText, int pX, int pY, const Font *pFont);
    void draw();
protected:
    const char *text;
    const Font *font;
};
