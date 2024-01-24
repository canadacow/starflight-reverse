#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <assert.h>

#include <array>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <future>

// Declare the promise and future as extern so they can be defined elsewhere
extern std::promise<void> initPromise;
extern std::future<void> initFuture;

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
    RunBitPixel,
};

enum IconType
{
    UnknownIcon = 0,
    Sun,
    Planet,
    Nebula,
    Flux,
    Ship,
};

// V= CONTEXT-ID#   ( 0=planet surface, 1=orbit, 2=system)         
// (3 = hyperspace, 4 = encounter, 5 = starport)
enum GameContext
{
    GC_PlanetSurface = 0,
    GC_Orbit = 1,
    GC_System = 2,
    GC_Hyperspace = 3,
    GC_Encounter = 4,
    GC_Starport = 5
};

#pragma pack(push, 1)

// Equivalent of UniformBlock struct
struct UniformBlock {
    int32_t graphics_mode_width;
    int32_t graphics_mode_height;
    int32_t window_width;
    int32_t window_height;
    uint32_t useRotoscope;
    uint32_t useEGA;
    float iTime; // Time in seconds from an arbitrary point in time
    float worldX;
    float worldY;
    float heading;
    float deadX;
    float deadY;
    // V= CONTEXT-ID#   ( 0=planet surface, 1=orbit, 2=system)         
    // (3 = hyperspace, 4 = encounter, 5 = starport)
    uint32_t game_context; 
    uint32_t alienVar1;
};

struct ShaderIcon {
    uint32_t isActive;
    int32_t x;
    int32_t y;
    int32_t screenX;

    int32_t screenY;
    int32_t bltX;
    int32_t bltY;
    uint32_t id;

    uint32_t clr;
    uint32_t icon_type;
    int32_t planet_to_sunX;
    int32_t planet_to_sunY;

    uint32_t planetIndex;
    uint32_t padding0;
    uint32_t padding1;
    uint32_t padding2;
};

struct Icon {
    int32_t x;
    int32_t y;
    int32_t screenX;
    int32_t screenY;

    int32_t bltX;
    int32_t bltY;
    uint32_t id;
    uint32_t clr;

    uint32_t icon_type;
    uint32_t iaddr;
    int32_t planet_to_sunX;
    int32_t planet_to_sunY;

    uint32_t seed;
};

extern std::vector<Icon> GetLocalIconList(uint32_t* gameContext);

struct IconUniform {
    ShaderIcon icons[32];

    IconUniform(std::vector<Icon> _icons)
    {
        assert(_icons.size() < _countof(icons));

        for(int i = 0; i < _countof(icons); ++i)
        {
            if(i < _icons.size())
            {
                auto& from = _icons.at(i);
                auto& to = icons[i];

                to.x = from.x;
                to.y = from.y;
                to.screenX = from.screenX;
                to.screenY = from.screenY;
                to.bltX = from.bltX;
                to.bltY = from.bltY;
                to.id = from.id;
                to.clr = from.clr;
                to.icon_type = from.icon_type;
                to.planet_to_sunX = from.planet_to_sunX;
                to.planet_to_sunY = from.planet_to_sunY;

                to.planetIndex = IndexFromSeed(from.seed);

                to.isActive = 1;
            }
            else
            {
                icons[i].isActive = 0;
            }
        }
    }

    uint32_t IndexFromSeed(uint32_t seed);
};

// Equivalent of TextData struct
struct TextData {
    uint32_t character;
    uint32_t xormode;
    uint32_t fontNum;
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

struct RunBitData {
    uint32_t tag;
};

struct Rotoscope;

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
    RunBitData runBitData;

    RotoscopeShader& operator=(const Rotoscope& other);
};

#pragma pack(pop)

struct PlanetSurface
{
    std::vector<uint8_t> relief;
    std::vector<uint32_t> albedo;
};

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
        RunBitData runBitData;
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
        bgColor = 0;
        fgColor = 0;
    }

    Rotoscope(PixelContents pixel)
    {
        assert(pixel == ClearPixel || pixel == PlotPixel || pixel == PolyFillPixel || pixel == TilePixel || pixel == EllipsePixel || pixel == RunBitPixel);
        content = pixel;
        EGAcolor = 0;
        argb = 0;
        blt_x = 0;
        blt_y = 0;
        blt_w = 0;
        blt_h = 0;
        bgColor = 0;
        fgColor = 0;
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
            bgColor = other.bgColor;
            fgColor = other.fgColor;

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
                case RunBitPixel:
                    runBitData = other.runBitData;
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
static const int NagivationWindowWidth = 72;
static const int NagivationWindowHeight = 120;

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

void GraphicsSetDeadReckoning(int16_t deadX, int16_t deadY);
void GraphicsReportGameFrame();

void GraphicsSplash(uint16_t seg, uint16_t fileNum);

void GraphicsInitPlanets(std::unordered_map<uint32_t, PlanetSurface> surfaces);

void WaitForVBlank();

void GraphicsCopyLine(uint16_t sourceSeg, uint16_t destSeg, uint16_t si, uint16_t di, uint16_t count);

void BeepOn();
void BeepTone(uint16_t pitFreq);
void BeepOff();

uint8_t GraphicsPeek(int x, int y, uint32_t offset, Rotoscope* pc = nullptr);
uint32_t GraphicsPeekDirect(int x, int y, uint32_t offset, Rotoscope* pc = nullptr);

int16_t GraphicsFONT(uint16_t num, uint32_t character, int x1, int y1, int color, int xormode, uint32_t offset);

#endif
