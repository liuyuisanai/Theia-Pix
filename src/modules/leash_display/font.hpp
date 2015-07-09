#pragma once

#include <stdio.h>

class Font
{
public:
    static Font LucideGrandeBig;
    static Font LucideGrandeMed;
    static Font LucideGrandeSmall;
    static Font LucideGrandeTiny;

    int getSymbolImageId(char c) const;
    size_t getTextWidth(const char *text);
    size_t getTextHeight(const char *text);
protected:
    Font(int pFirstSymbolImageId);
    int firstSymbolImageId;
};

