#ifndef _BLOCK_HPP_

#include <vector>
#include <string>

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


class MultiBlock : public Block {
public:
    ~MultiBlock();

    void add(Block *block);
    void draw();

protected:
    typedef std::vector<Block*> BlockVector;
    BlockVector blocks;
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
    TextBlock(const std::string &pText, int pX, int pY, int pDigitImageId, int pLetterImageId);
    void draw();
protected:
    std::string text;
    int digitImageId;
    int letterImageId;
};

#endif
