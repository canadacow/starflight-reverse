#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>


    extern uint32_t colortable[16];

    void GraphicsInit();
    void GraphicsUpdate();
    void GraphicsWait();
    void GraphicsQuit();

    bool GraphicsHasKey();
    uint16_t GraphicsGetKey();

    void GraphicsMode(int mode); // 0 = text, 1 = ega graphics
    void GraphicsClear(int color, uint32_t offset, int byteCount);
    void GraphicsText(char *s, int n);
    void GraphicsCarriageReturn();
    void GraphicsSetCursor(int x, int y);
    void GraphicsChar(unsigned char s);
    void GraphicsLine(int x1, int y1, int x2, int y2, int color, int xormode, uint32_t offset);
    void GraphicsPixel(int x, int y, int color, uint32_t offset);
    void GraphicsPixelDirect(int x, int y, uint32_t color, uint32_t offset);
    void GraphicsBLT(int x1, int y1, int w, int h, char* image, int color, int xormode, uint32_t offset);
    void GraphicsSave(char *filename);

    void GraphicsCopyLine(uint16_t sourceSeg, uint16_t destSeg, uint16_t si, uint16_t di, uint16_t count);

    void BeepOn();
    void BeepTone(uint16_t pitFreq);
    void BeepOff();

    uint8_t GraphicsPeek(int x, int y, uint32_t offset);
    uint32_t GraphicsPeekDirect(int x, int y, uint32_t offset);


#endif
