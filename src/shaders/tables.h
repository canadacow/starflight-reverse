// Equivalent of PixelContents enum
#define ClearPixel 0
#define NavigationalPixel 1
#define TextPixel 2
#define LinePixel 3
#define EllipsePixel 4
#define BoxFillPixel 5
#define PolyFillPixel 6
#define PicPixel 7
#define PlotPixel 8
#define TilePixel 9
#define RunBitPixel 10
#define AuxSysPixel 11

#define Icon_Other  0
#define Icon_Sun    1
#define Icon_Planet 2
#define Icon_Nebula 3

const vec4 colortable[16] = 
{
    vec4(0.0, 0.0, 0.0, 1.0), // black
    vec4(0.0, 0.0, 0.67, 1.0), // blue
    vec4(0.0, 0.67, 0.0, 1.0), // green
    vec4(0.0, 0.67, 0.67, 1.0), // cyan
    vec4(0.67, 0.0, 0.0, 1.0), // red
    vec4(0.67, 0.0, 0.67, 1.0), // magenta
    vec4(0.67, 0.33, 0.0, 1.0), // brown
    vec4(0.67, 0.67, 0.67, 1.0), // light gray
    vec4(0.33, 0.33, 0.33, 1.0), // dark gray
    vec4(0.33, 0.33, 1.0, 1.0),
    vec4(0.33, 1.0, 0.33, 1.0),
    vec4(0.33, 1.0, 1.0, 1.0),
    vec4(1.0, 0.33, 0.33, 1.0),
    vec4(1.0, 0.33, 1.0, 1.0),
    vec4(1.0, 1.0, 0.33, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0),
};

const vec4 CGAPalette[4] = 
{
    vec4(0.0, 0.0, 0.0, 1.0), // black
    vec4(0.0, 0.67, 0.67, 1.0), // cyan
    vec4(0.67, 0.0, 0.67, 1.0), // magenta
    vec4(0.67, 0.67, 0.67, 1.0), // light gray
};

const uint EGAToCGA[16] = {0, 2, 1, 11, 4, 6, 8, 10, 5, 3, 9, 7, 14, 0, 13, 15};