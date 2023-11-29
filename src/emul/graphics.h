#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <assert.h>

#include <array>
#include <utility>

enum PixelContents
{
    ClearPixel = 0,
    NavigationalPixel,
    TextPixel,
    LinePixel,
    EllipsePixel,
    BoxFillPixel,
    PolyFillPixel,
    PicPixel,
    PlotPixel,
    TilePixel,
    SplashPixel,
};

#pragma pack(push, 1)

// Equivalent of UniformBlock struct
struct UniformBlock {
    int32_t GRAPHICS_MODE_WIDTH;
    int32_t GRAPHICS_MODE_HEIGHT;
    int32_t WINDOW_WIDTH;
    int32_t WINDOW_HEIGHT;
    uint32_t useRotoscope;
    uint32_t useEGA;
};

// Equivalent of TextData struct
struct TextData {
    uint32_t character;
    uint32_t xormode;
    uint32_t fontNum;
    uint32_t fontWidth;
    uint32_t fontHeight;
};

// Equivalent of LineData struct
struct LineData {
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
    uint32_t n;
    uint32_t total;
};

// Equivalent of SplashData struct
struct SplashData {
    uint32_t seg;
    uint32_t fileNum;
};

// Equivalent of Rotoscope struct
struct RotoscopeShader {
    uint32_t content;
    uint32_t EGAcolor;
    uint32_t argb;
    int32_t blt_x;
    int32_t blt_y;
    int32_t blt_w;
    int32_t blt_h;
    uint32_t bgColor;
    uint32_t fgColor;
    TextData textData;
    LineData lineData;
    SplashData splashData;
};

#pragma pack(pop)

struct NavigationData
{
    uint8_t window_x;
    uint8_t window_y;
};

struct PicData
{
    uint64_t picID;
    uint8_t  pic_x;
    uint8_t  pic_y;
};

struct Rotoscope
{
    PixelContents content;

    uint8_t EGAcolor;

    uint32_t argb;

    int16_t blt_x;
    int16_t blt_y;

    int16_t blt_w;
    int16_t blt_h;

    uint8_t bgColor;
    uint8_t fgColor;

    union
    {
        NavigationData navigationData;
        TextData textData;
        PicData picData;
        LineData lineData;
        SplashData splashData;
    };

    Rotoscope()
    {
        content = ClearPixel;
        EGAcolor = 0;
        argb = 0;
        blt_x = 0;
        blt_y = 0;
        blt_w = 0;
        blt_h = 0;
    }

    Rotoscope(PixelContents pixel)
    {
        assert(pixel == ClearPixel || pixel == PlotPixel || pixel == PolyFillPixel || pixel == TilePixel || pixel == EllipsePixel || SplashPixel);
        content = pixel;
        EGAcolor = 0;
        argb = 0;
        blt_x = 0;
        blt_y = 0;
        blt_w = 0;
        blt_h = 0;
    }

    Rotoscope& operator=(const Rotoscope& other)
    {
        if (this != &other) // protect against invalid self-assignment
        {
            content = other.content;
            EGAcolor = other.EGAcolor;
            argb = other.argb;
            blt_x = other.blt_x;
            blt_y = other.blt_y;
            blt_w = other.blt_w;
            blt_h = other.blt_h;

            switch(content)
            {
                case ClearPixel:
                case EllipsePixel:
                case BoxFillPixel:
                case PlotPixel:
                case PolyFillPixel:
                case TilePixel:
                    break;
                case NavigationalPixel:
                    navigationData = other.navigationData;
                    break;
                case TextPixel:
                    assert(blt_w != 0);
                    assert(blt_h != 0);
                    textData = other.textData;
                    break;
                case PicPixel:
                    picData = other.picData;
                    break;
                case LinePixel:
                    lineData = other.lineData;
                    break;
                case SplashPixel:
                    splashData = other.splashData;
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        // by convention, always return *this
        return *this;
    }

    Rotoscope& operator=(const PixelContents& pixel) {
        assert(pixel == ClearPixel || pixel == PlotPixel || pixel == PolyFillPixel || pixel == TilePixel || pixel == EllipsePixel);
        content = pixel;
        EGAcolor = 0;
        argb = 0;
        blt_x = 0;
        blt_y = 0;
        return *this;
    }
};

static const int CGAToEGA[16] = {0, 2, 1, 9, 4, 8, 5, 11, 6, 10, 7, 3, 6, 14, 12, 15};
static const int EGAToCGA[16] = {0, 2, 1, 11, 4, 6, 8, 10, 5, 3, 9, 7, 14, 0, 13, 15};

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
void GraphicsPixel(int x, int y, int color, uint32_t offset, Rotoscope pc = Rotoscope(ClearPixel));
void GraphicsPixelDirect(int x, int y, uint32_t color, uint32_t offset, Rotoscope pc = Rotoscope(ClearPixel));
void GraphicsBLT(int16_t x1, int16_t y1, int16_t w, int16_t h, const char* image, int color, int xormode, uint32_t offset, Rotoscope pc = Rotoscope(ClearPixel));
void GraphicsSave(char *filename);

void WaitForVBlank();

void GraphicsCopyLine(uint16_t sourceSeg, uint16_t destSeg, uint16_t si, uint16_t di, uint16_t count);

void BeepOn();
void BeepTone(uint16_t pitFreq);
void BeepOff();

uint8_t GraphicsPeek(int x, int y, uint32_t offset, Rotoscope* pc = nullptr);
uint32_t GraphicsPeekDirect(int x, int y, uint32_t offset, Rotoscope* pc = nullptr);

int16_t GraphicsFONT(uint16_t num, uint32_t character, int x1, int y1, int color, int xormode, uint32_t offset);

#endif
