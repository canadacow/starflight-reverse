#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "graphics.h"
#include "../util/lodepng.h"

#include <vector>
#include <assert.h>
#include <algorithm>
#include <deque>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <semaphore>
#include <unordered_map>
#include <array>
#include <utility>

#define TEXT_MODE_WIDTH 640
#define TEXT_MODE_HEIGHT 200

#define GRAPHICS_MODE_WIDTH 160
#define GRAPHICS_MODE_HEIGHT 200
#define GRAPHICS_PAGE_COUNT 2
#define GRAPHICS_MEMORY_ALLOC 65536
static_assert(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT * GRAPHICS_PAGE_COUNT < GRAPHICS_MEMORY_ALLOC);

static uint32_t WINDOW_WIDTH = 1920;
static uint32_t WINDOW_HEIGHT = 1440;

static uint32_t OFFSCREEN_WINDOW_WIDTH = 960;
static uint32_t OFFSCREEN_WINDOW_HEIGHT = 720;

#define TARGET_RESOLUTION_WIDTH 3840
#define TARGET_RESOLUTION_HEIGHT 2160

#define ROTOSCOPE_MODE_WIDTH  160
#define ROTOSCOPE_MODE_HEIGHT 200

static SDL_Window *window = NULL;
static SDL_Renderer *renderer  = NULL;
static SDL_Texture* graphicsTexture = NULL;
static SDL_Texture* windowTexture = NULL;
static SDL_Texture* textTexture = NULL;

//#define ENABLE_OFFSCREEN_VIDEO_RENDERER 1

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
static SDL_Renderer *offscreenRenderer  = NULL;
static SDL_Texture* offscreenTexture = NULL;
static SDL_Window *offscreenWindow = NULL;
#endif

static SDL_AudioDeviceID audioDevice = 0;

#define FREQUENCY 48000 // Samples per second

static double toneInHz = 440.0;

static std::atomic<bool> s_useRotoscope = true;
static std::atomic<bool> s_useEGA = true;

union TextureColor {
    struct {
        float r, g, b, a;
    };
    float u[4];
};

uint32_t TextureColorToARGB(const TextureColor& color) {
    uint32_t a = static_cast<uint32_t>(color.a * 255.0f);
    uint32_t r = static_cast<uint32_t>(color.r * 255.0f);
    uint32_t g = static_cast<uint32_t>(color.g * 255.0f);
    uint32_t b = static_cast<uint32_t>(color.b * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
struct Texture {
    std::array<std::array<TextureColor, WIDTH>, HEIGHT> data;
};

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
TextureColor bilinearSample(const Texture<WIDTH, HEIGHT, PLANES>& texture, float u, float v) {
    u *= WIDTH - 1;
    v *= HEIGHT - 1;

    int x = (int)u;
    int y = (int)v;
    float u_ratio = u - x;
    float v_ratio = v - y;
    float u_opposite = 1 - u_ratio;
    float v_opposite = 1 - v_ratio;

    TextureColor result;
    for (std::size_t plane = 0; plane < PLANES; ++plane) {
        result.u[plane] = (texture.data[y][x].u[plane] * u_opposite + texture.data[y][x+1].u[plane] * u_ratio) * v_opposite +
                          (texture.data[y+1][x].u[plane] * u_opposite  + texture.data[y+1][x+1].u[plane] * u_ratio) * v_ratio;
    }
    return result;
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
void fillTexture(Texture<WIDTH, HEIGHT, PLANES>& texture, const std::vector<uint8_t>& image) {
    for(int i = 0; i < WIDTH; ++i) {
        for(int j = 0; j < HEIGHT; ++j) {
            for(int k = 0; k < PLANES; ++k) {
                texture.data[j][i].u[k] = static_cast<float>(image[(j * WIDTH + i) * PLANES + k]) / 255.0f;
            }
        }
    }
}

Texture<448, 160, 1> FONT1Texture;
Texture<840, 180, 1> FONT2Texture;
Texture<840, 220, 1> FONT3Texture;

enum SFGraphicsMode
{
    Text = 0,
    Graphics = 1,
};

SFGraphicsMode graphicsMode = Text;

uint32_t graphicsDisplayOffset = 0;// 100 * 160;
std::mutex rotoscopePixelMutex;
std::vector<Rotoscope> rotoscopePixels;
std::vector<uint32_t> graphicsPixels;
std::vector<uint32_t> textPixels;

std::jthread graphicsThread{};
std::binary_semaphore stopSemaphore{0};
std::mutex graphicsRetrace{};

int cursorx = 0;
int cursory = 0;

uint32_t colortable[16] =
{
0x000000, // black
0x0000AA, // blue
0x00AA00, // green
0x00AAAA, // cyan
0xAA0000, // red
0xAA00AA, // magenta
0xAA5500, // brown
0xAAAAAA, // light gray
0x555555, // dark gray
0x5555FF,
0x55FF55,
0x55FFFF,
0xFF5555,
0xFF55FF,
0xFFFF55,
0xFFFFFF,
};

static uint8_t vgafont8[256*8] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x7e,0x81,0xa5,0x81,0xbd,0x99,0x81,0x7e,
0x7e,0xff,0xdb,0xff,0xc3,0xe7,0xff,0x7e,
0x6c,0xfe,0xfe,0xfe,0x7c,0x38,0x10,0x00,
0x10,0x38,0x7c,0xfe,0x7c,0x38,0x10,0x00,
0x38,0x7c,0x38,0xfe,0xfe,0x7c,0x38,0x7c,
0x10,0x10,0x38,0x7c,0xfe,0x7c,0x38,0x7c,
0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,
0xff,0xff,0xe7,0xc3,0xc3,0xe7,0xff,0xff,
0x00,0x3c,0x66,0x42,0x42,0x66,0x3c,0x00,
0xff,0xc3,0x99,0xbd,0xbd,0x99,0xc3,0xff,
0x0f,0x07,0x0f,0x7d,0xcc,0xcc,0xcc,0x78,
0x3c,0x66,0x66,0x66,0x3c,0x18,0x7e,0x18,
0x3f,0x33,0x3f,0x30,0x30,0x70,0xf0,0xe0,
0x7f,0x63,0x7f,0x63,0x63,0x67,0xe6,0xc0,
0x99,0x5a,0x3c,0xe7,0xe7,0x3c,0x5a,0x99,
0x80,0xe0,0xf8,0xfe,0xf8,0xe0,0x80,0x00,
0x02,0x0e,0x3e,0xfe,0x3e,0x0e,0x02,0x00,
0x18,0x3c,0x7e,0x18,0x18,0x7e,0x3c,0x18,
0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x00,
0x7f,0xdb,0xdb,0x7b,0x1b,0x1b,0x1b,0x00,
0x3e,0x63,0x38,0x6c,0x6c,0x38,0xcc,0x78,
0x00,0x00,0x00,0x00,0x7e,0x7e,0x7e,0x00,
0x18,0x3c,0x7e,0x18,0x7e,0x3c,0x18,0xff,
0x18,0x3c,0x7e,0x18,0x18,0x18,0x18,0x00,
0x18,0x18,0x18,0x18,0x7e,0x3c,0x18,0x00,
0x00,0x18,0x0c,0xfe,0x0c,0x18,0x00,0x00,
0x00,0x30,0x60,0xfe,0x60,0x30,0x00,0x00,
0x00,0x00,0xc0,0xc0,0xc0,0xfe,0x00,0x00,
0x00,0x24,0x66,0xff,0x66,0x24,0x00,0x00,
0x00,0x18,0x3c,0x7e,0xff,0xff,0x00,0x00,
0x00,0xff,0xff,0x7e,0x3c,0x18,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x30,0x78,0x78,0x30,0x30,0x00,0x30,0x00,
0x6c,0x6c,0x6c,0x00,0x00,0x00,0x00,0x00,
0x6c,0x6c,0xfe,0x6c,0xfe,0x6c,0x6c,0x00,
0x30,0x7c,0xc0,0x78,0x0c,0xf8,0x30,0x00,
0x00,0xc6,0xcc,0x18,0x30,0x66,0xc6,0x00,
0x38,0x6c,0x38,0x76,0xdc,0xcc,0x76,0x00,
0x60,0x60,0xc0,0x00,0x00,0x00,0x00,0x00,
0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00,
0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00,
0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00,
0x00,0x30,0x30,0xfc,0x30,0x30,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,
0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,
0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x00,
0x7c,0xc6,0xce,0xde,0xf6,0xe6,0x7c,0x00,
0x30,0x70,0x30,0x30,0x30,0x30,0xfc,0x00,
0x78,0xcc,0x0c,0x38,0x60,0xcc,0xfc,0x00,
0x78,0xcc,0x0c,0x38,0x0c,0xcc,0x78,0x00,
0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x1e,0x00,
0xfc,0xc0,0xf8,0x0c,0x0c,0xcc,0x78,0x00,
0x38,0x60,0xc0,0xf8,0xcc,0xcc,0x78,0x00,
0xfc,0xcc,0x0c,0x18,0x30,0x30,0x30,0x00,
0x78,0xcc,0xcc,0x78,0xcc,0xcc,0x78,0x00,
0x78,0xcc,0xcc,0x7c,0x0c,0x18,0x70,0x00,
0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00,
0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x60,
0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x00,
0x00,0x00,0xfc,0x00,0x00,0xfc,0x00,0x00,
0x60,0x30,0x18,0x0c,0x18,0x30,0x60,0x00,
0x78,0xcc,0x0c,0x18,0x30,0x00,0x30,0x00,
0x7c,0xc6,0xde,0xde,0xde,0xc0,0x78,0x00,
0x30,0x78,0xcc,0xcc,0xfc,0xcc,0xcc,0x00,
0xfc,0x66,0x66,0x7c,0x66,0x66,0xfc,0x00,
0x3c,0x66,0xc0,0xc0,0xc0,0x66,0x3c,0x00,
0xf8,0x6c,0x66,0x66,0x66,0x6c,0xf8,0x00,
0xfe,0x62,0x68,0x78,0x68,0x62,0xfe,0x00,
0xfe,0x62,0x68,0x78,0x68,0x60,0xf0,0x00,
0x3c,0x66,0xc0,0xc0,0xce,0x66,0x3e,0x00,
0xcc,0xcc,0xcc,0xfc,0xcc,0xcc,0xcc,0x00,
0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
0x1e,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,0x00,
0xe6,0x66,0x6c,0x78,0x6c,0x66,0xe6,0x00,
0xf0,0x60,0x60,0x60,0x62,0x66,0xfe,0x00,
0xc6,0xee,0xfe,0xfe,0xd6,0xc6,0xc6,0x00,
0xc6,0xe6,0xf6,0xde,0xce,0xc6,0xc6,0x00,
0x38,0x6c,0xc6,0xc6,0xc6,0x6c,0x38,0x00,
0xfc,0x66,0x66,0x7c,0x60,0x60,0xf0,0x00,
0x78,0xcc,0xcc,0xcc,0xdc,0x78,0x1c,0x00,
0xfc,0x66,0x66,0x7c,0x6c,0x66,0xe6,0x00,
0x78,0xcc,0xe0,0x70,0x1c,0xcc,0x78,0x00,
0xfc,0xb4,0x30,0x30,0x30,0x30,0x78,0x00,
0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xfc,0x00,
0xcc,0xcc,0xcc,0xcc,0xcc,0x78,0x30,0x00,
0xc6,0xc6,0xc6,0xd6,0xfe,0xee,0xc6,0x00,
0xc6,0xc6,0x6c,0x38,0x38,0x6c,0xc6,0x00,
0xcc,0xcc,0xcc,0x78,0x30,0x30,0x78,0x00,
0xfe,0xc6,0x8c,0x18,0x32,0x66,0xfe,0x00,
0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00,
0xc0,0x60,0x30,0x18,0x0c,0x06,0x02,0x00,
0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00,
0x10,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x78,0x0c,0x7c,0xcc,0x76,0x00,
0xe0,0x60,0x60,0x7c,0x66,0x66,0xdc,0x00,
0x00,0x00,0x78,0xcc,0xc0,0xcc,0x78,0x00,
0x1c,0x0c,0x0c,0x7c,0xcc,0xcc,0x76,0x00,
0x00,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0x38,0x6c,0x60,0xf0,0x60,0x60,0xf0,0x00,
0x00,0x00,0x76,0xcc,0xcc,0x7c,0x0c,0xf8,
0xe0,0x60,0x6c,0x76,0x66,0x66,0xe6,0x00,
0x30,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x0c,0x00,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,
0xe0,0x60,0x66,0x6c,0x78,0x6c,0xe6,0x00,
0x70,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
0x00,0x00,0xcc,0xfe,0xfe,0xd6,0xc6,0x00,
0x00,0x00,0xf8,0xcc,0xcc,0xcc,0xcc,0x00,
0x00,0x00,0x78,0xcc,0xcc,0xcc,0x78,0x00,
0x00,0x00,0xdc,0x66,0x66,0x7c,0x60,0xf0,
0x00,0x00,0x76,0xcc,0xcc,0x7c,0x0c,0x1e,
0x00,0x00,0xdc,0x76,0x66,0x60,0xf0,0x00,
0x00,0x00,0x7c,0xc0,0x78,0x0c,0xf8,0x00,
0x10,0x30,0x7c,0x30,0x30,0x34,0x18,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0xcc,0x76,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0x78,0x30,0x00,
0x00,0x00,0xc6,0xd6,0xfe,0xfe,0x6c,0x00,
0x00,0x00,0xc6,0x6c,0x38,0x6c,0xc6,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0x7c,0x0c,0xf8,
0x00,0x00,0xfc,0x98,0x30,0x64,0xfc,0x00,
0x1c,0x30,0x30,0xe0,0x30,0x30,0x1c,0x00,
0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00,
0xe0,0x30,0x30,0x1c,0x30,0x30,0xe0,0x00,
0x76,0xdc,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0x00,
0x78,0xcc,0xc0,0xcc,0x78,0x18,0x0c,0x78,
0x00,0xcc,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x1c,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0x7e,0xc3,0x3c,0x06,0x3e,0x66,0x3f,0x00,
0xcc,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0xe0,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x30,0x30,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x00,0x00,0x78,0xc0,0xc0,0x78,0x0c,0x38,
0x7e,0xc3,0x3c,0x66,0x7e,0x60,0x3c,0x00,
0xcc,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0xe0,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0xcc,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x7c,0xc6,0x38,0x18,0x18,0x18,0x3c,0x00,
0xe0,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0xc6,0x38,0x6c,0xc6,0xfe,0xc6,0xc6,0x00,
0x30,0x30,0x00,0x78,0xcc,0xfc,0xcc,0x00,
0x1c,0x00,0xfc,0x60,0x78,0x60,0xfc,0x00,
0x00,0x00,0x7f,0x0c,0x7f,0xcc,0x7f,0x00,
0x3e,0x6c,0xcc,0xfe,0xcc,0xcc,0xce,0x00,
0x78,0xcc,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0xcc,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0xe0,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x78,0xcc,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xe0,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xcc,0x00,0xcc,0xcc,0x7c,0x0c,0xf8,
0xc3,0x18,0x3c,0x66,0x66,0x3c,0x18,0x00,
0xcc,0x00,0xcc,0xcc,0xcc,0xcc,0x78,0x00,
0x18,0x18,0x7e,0xc0,0xc0,0x7e,0x18,0x18,
0x38,0x6c,0x64,0xf0,0x60,0xe6,0xfc,0x00,
0xcc,0xcc,0x78,0xfc,0x30,0xfc,0x30,0x30,
0xf8,0xcc,0xcc,0xfa,0xc6,0xcf,0xc6,0xc7,
0x0e,0x1b,0x18,0x3c,0x18,0x18,0xd8,0x70,
0x1c,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x38,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x00,0x1c,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0x1c,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xf8,0x00,0xf8,0xcc,0xcc,0xcc,0x00,
0xfc,0x00,0xcc,0xec,0xfc,0xdc,0xcc,0x00,
0x3c,0x6c,0x6c,0x3e,0x00,0x7e,0x00,0x00,
0x38,0x6c,0x6c,0x38,0x00,0x7c,0x00,0x00,
0x30,0x00,0x30,0x60,0xc0,0xcc,0x78,0x00,
0x00,0x00,0x00,0xfc,0xc0,0xc0,0x00,0x00,
0x00,0x00,0x00,0xfc,0x0c,0x0c,0x00,0x00,
0xc3,0xc6,0xcc,0xde,0x33,0x66,0xcc,0x0f,
0xc3,0xc6,0xcc,0xdb,0x37,0x6f,0xcf,0x03,
0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x00,
0x00,0x33,0x66,0xcc,0x66,0x33,0x00,0x00,
0x00,0xcc,0x66,0x33,0x66,0xcc,0x00,0x00,
0x22,0x88,0x22,0x88,0x22,0x88,0x22,0x88,
0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,
0xdb,0x77,0xdb,0xee,0xdb,0x77,0xdb,0xee,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xf8,0x18,0x18,0x18,
0x18,0x18,0xf8,0x18,0xf8,0x18,0x18,0x18,
0x36,0x36,0x36,0x36,0xf6,0x36,0x36,0x36,
0x00,0x00,0x00,0x00,0xfe,0x36,0x36,0x36,
0x00,0x00,0xf8,0x18,0xf8,0x18,0x18,0x18,
0x36,0x36,0xf6,0x06,0xf6,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
0x00,0x00,0xfe,0x06,0xf6,0x36,0x36,0x36,
0x36,0x36,0xf6,0x06,0xfe,0x00,0x00,0x00,
0x36,0x36,0x36,0x36,0xfe,0x00,0x00,0x00,
0x18,0x18,0xf8,0x18,0xf8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xf8,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x1f,0x00,0x00,0x00,
0x18,0x18,0x18,0x18,0xff,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x1f,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,
0x18,0x18,0x18,0x18,0xff,0x18,0x18,0x18,
0x18,0x18,0x1f,0x18,0x1f,0x18,0x18,0x18,
0x36,0x36,0x36,0x36,0x37,0x36,0x36,0x36,
0x36,0x36,0x37,0x30,0x3f,0x00,0x00,0x00,
0x00,0x00,0x3f,0x30,0x37,0x36,0x36,0x36,
0x36,0x36,0xf7,0x00,0xff,0x00,0x00,0x00,
0x00,0x00,0xff,0x00,0xf7,0x36,0x36,0x36,
0x36,0x36,0x37,0x30,0x37,0x36,0x36,0x36,
0x00,0x00,0xff,0x00,0xff,0x00,0x00,0x00,
0x36,0x36,0xf7,0x00,0xf7,0x36,0x36,0x36,
0x18,0x18,0xff,0x00,0xff,0x00,0x00,0x00,
0x36,0x36,0x36,0x36,0xff,0x00,0x00,0x00,
0x00,0x00,0xff,0x00,0xff,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0xff,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0x3f,0x00,0x00,0x00,
0x18,0x18,0x1f,0x18,0x1f,0x00,0x00,0x00,
0x00,0x00,0x1f,0x18,0x1f,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0x3f,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0xff,0x36,0x36,0x36,
0x18,0x18,0xff,0x18,0xff,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xf8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1f,0x18,0x18,0x18,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
0x00,0x00,0x76,0xdc,0xc8,0xdc,0x76,0x00,
0x00,0x78,0xcc,0xf8,0xcc,0xf8,0xc0,0xc0,
0x00,0xfc,0xcc,0xc0,0xc0,0xc0,0xc0,0x00,
0x00,0xfe,0x6c,0x6c,0x6c,0x6c,0x6c,0x00,
0xfc,0xcc,0x60,0x30,0x60,0xcc,0xfc,0x00,
0x00,0x00,0x7e,0xd8,0xd8,0xd8,0x70,0x00,
0x00,0x66,0x66,0x66,0x66,0x7c,0x60,0xc0,
0x00,0x76,0xdc,0x18,0x18,0x18,0x18,0x00,
0xfc,0x30,0x78,0xcc,0xcc,0x78,0x30,0xfc,
0x38,0x6c,0xc6,0xfe,0xc6,0x6c,0x38,0x00,
0x38,0x6c,0xc6,0xc6,0x6c,0x6c,0xee,0x00,
0x1c,0x30,0x18,0x7c,0xcc,0xcc,0x78,0x00,
0x00,0x00,0x7e,0xdb,0xdb,0x7e,0x00,0x00,
0x06,0x0c,0x7e,0xdb,0xdb,0x7e,0x60,0xc0,
0x38,0x60,0xc0,0xf8,0xc0,0x60,0x38,0x00,
0x78,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x00,
0x00,0xfc,0x00,0xfc,0x00,0xfc,0x00,0x00,
0x30,0x30,0xfc,0x30,0x30,0x00,0xfc,0x00,
0x60,0x30,0x18,0x30,0x60,0x00,0xfc,0x00,
0x18,0x30,0x60,0x30,0x18,0x00,0xfc,0x00,
0x0e,0x1b,0x1b,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0xd8,0xd8,0x70,
0x30,0x30,0x00,0xfc,0x00,0x30,0x30,0x00,
0x00,0x76,0xdc,0x00,0x76,0xdc,0x00,0x00,
0x38,0x6c,0x6c,0x38,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,
0x0f,0x0c,0x0c,0x0c,0xec,0x6c,0x3c,0x1c,
0x78,0x6c,0x6c,0x6c,0x6c,0x00,0x00,0x00,
0x70,0x18,0x30,0x60,0x78,0x00,0x00,0x00,
0x00,0x00,0x3c,0x3c,0x3c,0x3c,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

class DOSKeyboard {
public:
    // Destructive read equivalent to Int 16 ah = 0
    virtual bool checkForKeyStroke() = 0;
    // Destructive read equivalent to Int 16 ah = 0
    virtual unsigned short getKeyStroke() = 0;

    virtual ~DOSKeyboard() = default;
};

class CLIKeyboard : public DOSKeyboard {
private:
    std::deque<uint16_t> queuedString{};

public:
    CLIKeyboard() = default;

    virtual ~CLIKeyboard() = default;

    bool checkForKeyStroke() override {
        return true;
    }

    unsigned short getKeyStroke() override {
        if(queuedString.empty())
        {
            printf("input: ");
            fflush(stdout);
            int c;
            do {
                c = getchar();
                queuedString.push_back(c);
            } while(c != '\n');
        }

        auto ret = queuedString.front();
        queuedString.pop_front();

        return ret;
    }
};

class SDLKeyboard : public DOSKeyboard {
private:
    std::deque<SDL_Event> eventQueue{};

public:
    SDLKeyboard() {}

    static unsigned short GetKey(int sym)
    {
        if (sym == SDLK_LEFT)
        {
            return 331;
        }
        if (sym == SDLK_RIGHT)
        {
            return 333;
        }
        if (sym == SDLK_UP)
        {
            return 328;
        }
        if (sym == SDLK_DOWN)
        {
            return 336;
        }
        if (sym == SDLK_KP_8)
        {
            return 328;
        }
        if (sym == SDLK_KP_2)
        {
            return 336;
        }
        if (sym == SDLK_KP_4)
        {
            return 331;
        }
        if (sym == SDLK_KP_6)
        {
            return 333;
        }
        if (sym == SDLK_KP_7)
        {
            return 327;
        }
        if (sym == SDLK_KP_9)
        {
            return 329;
        }
        if (sym == SDLK_KP_1)
        {
            return 335;
        }
        if (sym == SDLK_KP_3)
        {
            return 337;
        }

        return sym;
    }

    void update(bool blocking) {

        auto handleEvent = [&](SDL_Event event) -> bool {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_F1)
                    {
                        s_useRotoscope = !s_useRotoscope;
                    }
                    else if(event.key.keysym.sym == SDLK_F2)
                    {
                        s_useEGA = !s_useEGA;
                    }
                    else if(eventQueue.size() < 4)
                    {
                        eventQueue.push_back(event);
                    }
                    return true;
                    break;
                case SDL_WINDOWEVENT:
                    {
                        if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        {
                            if(SDL_GetWindowFromID(event.window.windowID) == window)
                            {
                                GraphicsQuit();
                                exit(0);
                            }
                            #if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
                            if(SDL_GetWindowFromID(event.window.windowID) == offscreenWindow)
                            {
                                SDL_DestroyWindow(offscreenWindow);
                                offscreenWindow = nullptr;
                            }
                            #endif
                        }
                    }
                    break;
                case SDL_QUIT:
                    GraphicsQuit();
                    exit(0);
                    break;
                default:
                    break;
            }

            return false;
        };

        if(blocking)
        {
            SDL_Event event;
            for(;;)
            {
                if(SDL_WaitEvent(&event))
                {
                    bool key = handleEvent(event);
                    if(key)
                        break;
                }
                else
                {
                    break;
                }
                std::this_thread::yield();
            }
            
        }
        else
        {
            SDL_Event event;
            if(SDL_PollEvent(&event))
            {
                handleEvent(event);
            }
            std::this_thread::yield();
        }
    }

    // Non-destructive read equivalent to Int 16 ah = 1
    bool checkForKeyStroke() override {
        update(false);
        return !eventQueue.empty();
    }

    // Destructive read equivalent to Int 16 ah = 0
    unsigned short getKeyStroke() override {
        if (eventQueue.empty())
        {
            update(true);
        }

        SDL_Event event = eventQueue.front();
        eventQueue.pop_front();
        return GetKey(event.key.keysym.sym);
    }
};

static std::unique_ptr<DOSKeyboard> keyboard{};

static bool s_audioPlaying = false;

void play_buffer(void*, unsigned char*, int);

SDL_AudioSpec spec = {
	.freq = FREQUENCY, 
	.format = AUDIO_S16SYS, // Signed 16 bit integer format
	.channels = 1,
	.samples = 512, // The size of each "chunk"
	.callback = play_buffer, // user-defined function that provides the audio data
	.userdata = NULL // an argument to the callback function (we dont need any)
};

// Generate a sine wave
double tone(double hz, unsigned long time) {
	return sin(time * hz * M_PI * 2 / FREQUENCY);
}

// Generate a sawtooth wave
double saw(double hz, unsigned long time) {
	return fmod(time*hz/FREQUENCY, 1)*2-1;
}

// Generate a square wave
double square(double hz, unsigned long time) {
	double sine = tone(hz, time);
	return sine > 0.0 ? 1.0 : -1.0;
}

// This is the function that gets automatically called every time the audio device needs more data
void play_buffer(void* userdata, unsigned char* stream, int len) {
	SDL_memset(stream, spec.silence, len);

    static unsigned long time = 0;
    Sint16 *stream16 = (Sint16*)stream;
    for(int i = 0; i < len/2; i++, time++) {
        if(s_audioPlaying)
        {
            stream16[i] = (Sint16)(square(toneInHz, time) * 5000);
        }
        else
        {
            stream16[i] = spec.silence;
        }
        
    }
}

void BeepOn()
{
    SDL_PauseAudioDevice(audioDevice, 0);
    s_audioPlaying = true;
}

void BeepTone(uint16_t pitFreq)
{
    toneInHz = 1193182.0 / pitFreq;
}

void BeepOff()
{
    s_audioPlaying = false;
}

static int GraphicsInitThread(void *ptr)
{
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
    }
    else
    {
        SDL_Log("Desktop display mode: %dx%dpx @ %dhz", dm.w, dm.h, dm.refresh_rate);
    }

    WINDOW_WIDTH = (WINDOW_WIDTH * dm.w) / TARGET_RESOLUTION_WIDTH;
    WINDOW_HEIGHT = (WINDOW_HEIGHT * dm.h) / TARGET_RESOLUTION_HEIGHT;
    OFFSCREEN_WINDOW_WIDTH = (OFFSCREEN_WINDOW_WIDTH * dm.w) / TARGET_RESOLUTION_WIDTH;
    OFFSCREEN_WINDOW_HEIGHT = (OFFSCREEN_WINDOW_HEIGHT * dm.h) / TARGET_RESOLUTION_HEIGHT;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    window = SDL_CreateWindow("Starflight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }  

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    graphicsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
    if (graphicsTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    windowTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (windowTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // Create the text mode texture
    textTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TEXT_MODE_WIDTH, TEXT_MODE_HEIGHT);
    if (textTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)

    offscreenWindow = SDL_CreateWindow("Off Screen Starflight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, OFFSCREEN_WINDOW_WIDTH, OFFSCREEN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (offscreenWindow == NULL)
    {
        printf("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    offscreenRenderer = SDL_CreateRenderer(offscreenWindow, -1, 0);
    if (offscreenRenderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    offscreenTexture = SDL_CreateTexture(offscreenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
    if (offscreenTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

#endif

    keyboard = std::make_unique<SDLKeyboard>();

    graphicsPixels = std::vector<uint32_t>();
    graphicsPixels.resize(GRAPHICS_MEMORY_ALLOC);

    rotoscopePixels = std::vector<Rotoscope>();
    rotoscopePixels.resize(GRAPHICS_MEMORY_ALLOC);

    textPixels = std::vector<uint32_t>();
    textPixels.resize(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT);

    std::vector<uint8_t> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, "FONT1_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT1Texture, image);
    image.clear();

    error = lodepng::decode(image, width, height, "FONT2_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT2Texture, image);
    image.clear();

    error = lodepng::decode(image, width, height, "FONT3_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT3Texture, image);
    image.clear();

    return 0;
}

void GraphicsInit()
{
#ifdef SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }
    FILE* file;
    file = freopen("stdout", "w", stdout); // redirects stdout
    file = freopen("stderr", "w", stderr); // redirects stderr
#endif

    GraphicsInitThread(NULL);
}

template<typename T>
struct vec2 {
    union {
        struct {
            T x;
            T y;
        };
        struct {
            T u;
            T v;
        };
    };
};

uint32_t DrawLinePixel(const Rotoscope& roto, vec2<float> uv, float polygonWidth)
{
    uint32_t pixel = 0;

    float lineX1 = ((float)roto.lineData.x0 + 0.10f)  / (float)GRAPHICS_MODE_WIDTH;
    float lineY1 = ((float)roto.lineData.y0 + 0.10f) / (float)GRAPHICS_MODE_HEIGHT;
    float lineX2 = ((float)roto.lineData.x1 + 0.90f) / (float)GRAPHICS_MODE_WIDTH;
    float lineY2 = ((float)roto.lineData.y1 + 0.90f) / (float)GRAPHICS_MODE_HEIGHT;

    float a = polygonWidth;
    float one_px = 1.0f / WINDOW_WIDTH;
    vec2<float> p1 = {lineX1, lineY1};
    vec2<float> p2 = {lineX2, lineY2};
    auto mix = [](float a, float b, float t) { return a * (1 - t) + b * t; };
    auto distance = [](vec2<float> a, vec2<float> b) { return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2)); };

    float d = distance(p1, p2);
    float duv = distance(p1, uv);

    float r = 1.0f - floor(1.0f - (a * one_px) + distance(vec2<float>{mix(p1.x, p2.x, std::clamp(duv / d, 0.0f, 1.0f)), mix(p1.y, p2.y, std::clamp(duv / d, 0.0f, 1.0f))}, uv));

    if (r > 0.0f)
    {
        pixel = colortable[roto.lineData.fgColor & 0xf];
    }
    else
    {
        pixel = colortable[roto.lineData.bgColor & 0xf];
    }

    return pixel;
}

uint32_t DrawFontPixel(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv)
{
    uint32_t pixel = 0;

    float fontX = (float)roto.blt_x / (float)roto.blt_w;
    float fontY = (float)roto.blt_y / (float)roto.blt_h;
    
    subUv.u /= (float)roto.textData.fontWidth;
    subUv.v /= (float)roto.textData.fontHeight;

    fontX += subUv.u;
    fontY += subUv.v;

    if(roto.textData.fontNum == 1)
    {
        // Find the character in our atlas.
        constexpr float fontSpaceWidth = 8.0f * 4.0f;
        constexpr float fontSpaceHeight = 8.0f * 4.0f;

        constexpr float atlasWidth = 448.0f;
        constexpr float atlasHeight = 160.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 448 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT1Texture, u, v);
        pixel = colortable[roto.textData.bgColor & 0xf];
        if(glyph.r > 0.80f)
        {
            pixel = colortable[roto.textData.fgColor & 0xf];
        }
        #if 0
        else
        {
            #if 1
            uint32_t r = static_cast<uint32_t>(fontX * 255);
            uint32_t g = static_cast<uint32_t>(fontY * 255);
            uint32_t b = 0;
            uint32_t a = 255;

            pixel = (a << 24) | (r << 16) | (g << 8) | b;
            #endif
        }
        #endif
    } else if (roto.textData.fontNum == 2)
    {
        // Find the character in our atlas.
        constexpr float fontSpaceWidth = 15.0f * 4.0f;
        constexpr float fontSpaceHeight = 11.0f * 4.0f;

        constexpr float atlasWidth = 840.0f;
        constexpr float atlasHeight = 220.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 840 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT3Texture, u, v);
        pixel = colortable[roto.textData.bgColor & 0xf];
        if(glyph.r > 0.9f)
        {
            pixel = colortable[roto.textData.fgColor & 0xf];
        }
    } else if (roto.textData.fontNum == 3)
    {
                            // Find the character in our atlas.
        constexpr float fontSpaceWidth = 15.0f * 4.0f;
        constexpr float fontSpaceHeight = 11.0f * 4.0f;

        constexpr float atlasWidth = 840.0f;
        constexpr float atlasHeight = 220.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 840 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT3Texture, u, v);
        pixel = colortable[roto.textData.bgColor & 0xf];
        if(glyph.r > 0.9f)
        {
            pixel = colortable[roto.textData.fgColor & 0xf];
        }
        #if 0
        else
        {
            uint32_t r = static_cast<uint32_t>(fontX * 255);
            uint32_t g = static_cast<uint32_t>(fontY * 255);
            uint32_t b = 0;
            uint32_t a = 255;

            pixel = (a << 24) | (r << 16) | (g << 8) | b;
        }
        #endif
    }
    #if 0
    else
    {
        uint32_t r = static_cast<uint32_t>(fontX * 255);
        uint32_t g = static_cast<uint32_t>(fontY * 255);
        uint32_t b = 0;
        uint32_t a = 255;

        pixel = (a << 24) | (r << 16) | (g << 8) | b;
    }
    #endif

    return pixel;
}

uint32_t DrawSplashPixel(const Rotoscope& roto, vec2<float> uv)
{
    static Texture<1536, 1152, 4> LOGO1Texture;
    static Texture<1536, 1152, 4> LOGO2Texture;
    static bool loaded = false;

    uint32_t pixel = 0;

    if(!loaded)
    {
        loaded = true;
        std::vector<uint8_t> image;
        unsigned width, height;

        unsigned error = lodepng::decode(image, width, height, "logo_1.png", LCT_RGBA, 8);
        if(error) 
        {
            printf("decoder error %d, %s\n", error, lodepng_error_text(error));
            exit(-1);
        }

        fillTexture(LOGO1Texture, image);
        image.clear();

        error = lodepng::decode(image, width, height, "logo_2.png", LCT_RGBA, 8);
        if(error) 
        {
            printf("decoder error %d, %s\n", error, lodepng_error_text(error));
            exit(-1);
        }

        fillTexture(LOGO2Texture, image);
        image.clear();
    }

    switch(roto.splashData.fileNum)
    {
        case 0x008d: // First splash
            pixel = TextureColorToARGB(bilinearSample(LOGO1Texture, uv.u, uv.v));
            break;
        case 0x0036: // Second splash
            pixel = TextureColorToARGB(bilinearSample(LOGO2Texture, uv.u, uv.v));
            break;
        default:
            assert(false);
            break;
    }

    return pixel;
}

void DoRotoscope(std::vector<uint32_t>& windowData, const std::vector<Rotoscope>& rotoPixels)
{
    uint32_t index = 0;
    const float polygonWidth = (float)WINDOW_WIDTH / (float)GRAPHICS_MODE_WIDTH;

    constexpr CGAToEGAMap map;

    for(uint32_t y = 0; y < WINDOW_HEIGHT; ++y)
    {
        for(uint32_t x = 0; x < WINDOW_WIDTH; ++x)
        {
            // Calculate the corresponding position in the smaller texture
            uint32_t srcX = x * GRAPHICS_MODE_WIDTH / WINDOW_WIDTH;
            uint32_t srcY = y * GRAPHICS_MODE_HEIGHT / WINDOW_HEIGHT;

            // Calculate the index in the smaller texture
            uint32_t srcIndex = srcY * GRAPHICS_MODE_WIDTH + srcX;

            auto& roto = rotoPixels[srcIndex];

            float xcoord = (float)x / (float)WINDOW_WIDTH;
            float ycoord = (float)y / (float)WINDOW_HEIGHT;
            vec2<float> uv = {xcoord, ycoord};

            float subPixelXOffset = (xcoord * GRAPHICS_MODE_WIDTH) - srcX;
            float subPixelYOffset = (ycoord * GRAPHICS_MODE_HEIGHT) - srcY;
            vec2<float> subUv = {subPixelXOffset, subPixelYOffset};

            // Pull the pixel from the smaller texture
            uint32_t pixel = roto.argb;

            if(s_useRotoscope)
            {
                switch(roto.content)
                {
                    case LinePixel:
                        //pixel = DrawLinePixel(roto, uv, polygonWidth);
                        break;
                    case TextPixel:
                        pixel = DrawFontPixel(roto, uv, subUv);    
                        break;
                    case SplashPixel:
                        pixel = DrawSplashPixel(roto, uv);
                        break;
                    default:
                        //pixel = 0xffff0000;
                        //pixel = colortable[(int)roto.content];
                        break;
                }
            }
            else
            {
                if(!s_useEGA)
                {
                    static uint32_t CGAPalette[4] = {0xff000000, 0xff00aaaa, 0xffaa00aa, 0xffaaaaaa};

                    uint8_t c = map.getEGAToCGA(roto.EGAcolor);
                    if(subUv.u < 0.5f)
                    {
                        pixel = CGAPalette[(c >> 2) & 3];
                    }
                    else
                    {
                        pixel = CGAPalette[c & 3];
                    }
                }
            }

            // Place the pixel in the larger surface
            windowData[index] = pixel;
            ++index;
        }
    }
}

void GraphicsUpdate()
{
    SDL_Texture* currentTexture = NULL;
    uint32_t stride = 0;
    const void* data = nullptr;

    static std::vector<uint32_t> fullRes{};
    static std::vector<Rotoscope> backbuffer{};

    if(fullRes.size() == 0)
    {
        fullRes.resize(WINDOW_WIDTH * WINDOW_HEIGHT);
        backbuffer.resize(GRAPHICS_MODE_WIDTH *GRAPHICS_MODE_HEIGHT);
    }

    // Choose the correct texture based on the current mode
    if (graphicsMode == SFGraphicsMode::Graphics)
    {
    #if 0
        currentTexture = graphicsTexture;
        stride = GRAPHICS_MODE_WIDTH;
        data = graphicsPixels.data() + graphicsDisplayOffset;
    #else
        {
            std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

            for(int i = 0; i < GRAPHICS_MODE_WIDTH *GRAPHICS_MODE_HEIGHT; ++i)
            {
                backbuffer[i] = rotoscopePixels[i];
            }
        }

#if 0
        static int frameCount = 0;
        std::string filename = "frame_" + std::to_string(frameCount++) + ".png";
        std::vector<unsigned char> png;
        unsigned width = GRAPHICS_MODE_WIDTH, height = GRAPHICS_MODE_HEIGHT;
        std::vector<unsigned char> image;
        image.resize(width * height * 4);
        for(unsigned y = 0; y < height; y++)
        {
            for(unsigned x = 0; x < width; x++)
            {
                uint32_t pixel = graphicsPixels[y * width + x];
                image[4 * width * y + 4 * x + 0] = (pixel >> 16) & 0xFF; // R
                image[4 * width * y + 4 * x + 1] = (pixel >> 8) & 0xFF; // G
                image[4 * width * y + 4 * x + 2] = pixel & 0xFF; // B
                image[4 * width * y + 4 * x + 3] = 255; // A
            }
        }
        unsigned error = lodepng::encode(filename, image, width, height);
        if(error)
        {
            printf("encoder error %u: %s\n", error, lodepng_error_text(error));
        }     
#endif    

        DoRotoscope(fullRes, backbuffer);
        currentTexture = windowTexture;
        stride = WINDOW_WIDTH;
        data = fullRes.data();
    #endif
    }
    else if (graphicsMode == SFGraphicsMode::Text)
    {
        currentTexture = textTexture;
        stride = TEXT_MODE_WIDTH;
        data = textPixels.data();
    }

    SDL_UpdateTexture(currentTexture, NULL, data, stride * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, currentTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_PumpEvents();

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
    if(graphicsMode == SFGraphicsMode::Graphics)
    {
        data = (uint8_t*)graphicsPixels.data() + 0x20000;

        stride = GRAPHICS_MODE_WIDTH;

        SDL_UpdateTexture(offscreenTexture, NULL, data, stride * sizeof(uint32_t));
        SDL_RenderClear(offscreenRenderer);
        SDL_RenderCopy(offscreenRenderer, offscreenTexture, NULL, NULL);
        SDL_RenderPresent(offscreenRenderer);
    }
#endif
}

void GraphicsWait()
{
    SDL_Event event;

    while(1)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                return;
        }
    }
}

void GraphicsQuit()
{
    if(graphicsThread.joinable())
    {
        stopSemaphore.release();

        graphicsThread.join();
    }

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
    SDL_DestroyRenderer(offscreenRenderer);
    SDL_DestroyWindow(offscreenWindow);
    SDL_DestroyTexture(offscreenTexture);
#endif
    
    SDL_DestroyTexture(graphicsTexture);
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void GraphicsSetCursor(int x, int y)
{
    cursorx = x;
    cursory = y;
}

void GraphicsCarriageReturn()
{
    cursorx = 0;
    cursory++;
}

void GraphicsChar(unsigned char s)
{
    assert(graphicsMode == Text);
    for(int jj=0; jj<8; jj++)
    {
        int offset = ((int)s)*8 + jj;
        for(int ii=0; ii<8; ii++)
        {
            int color = 0;
            if ((vgafont8[offset]) & (1<<(7-ii)))
            {
                color = 0xFFFFFFFF;
            }

            textPixels[(cursory*8+jj) * TEXT_MODE_WIDTH + (cursorx*8+ii)] = color;
        }
    }

    cursorx++;
    if (cursorx >=80)
    {
        GraphicsCarriageReturn();
    }
}

void GraphicsText(char *s, int n)
{
    for(int i=0; i<n; i++)
    {
        GraphicsChar(s[i]);
    }
}

// 0 = text, 1 = ega graphics
void GraphicsMode(int mode)
{
    if(graphicsThread.joinable())
    {
        stopSemaphore.release();

        graphicsThread.join();
    }

    graphicsMode = (SFGraphicsMode)mode;

    std::fill(graphicsPixels.begin(), graphicsPixels.end(), 0);
    std::fill(textPixels.begin(), textPixels.end(), 0);
    std::fill(rotoscopePixels.begin(), rotoscopePixels.end(), ClearPixel);

    graphicsThread = std::jthread([]{
        while(!stopSemaphore.try_acquire()) {
            constexpr std::chrono::nanoseconds scanout_duration = std::chrono::nanoseconds(13340000); // 80% of 1/60th of a second
            constexpr std::chrono::nanoseconds retrace_duration = std::chrono::nanoseconds(3330000); // 20% of 1/60th of a second
            {
                std::lock_guard<std::mutex> lg(graphicsRetrace);
                GraphicsUpdate();

                std::this_thread::sleep_for(scanout_duration);
            }
            std::this_thread::sleep_for(retrace_duration);
        }
    });
}

void WaitForVBlank()
{
    graphicsRetrace.lock();
    graphicsRetrace.unlock();
}

void GraphicsClear(int color, uint32_t offset, int byteCount)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);
    uint32_t dest = (uint32_t)offset;

    dest <<= 4; // Convert to linear addres
    dest -= 0xa0000; // Subtract from EGA page
    dest *= 4; // Convert to our SDL memory linear address

    uint32_t destOffset = 0;

    auto c = colortable[color&0xF];

    byteCount = 0x2000;

    for(uint32_t i = 0; i < byteCount * 4; ++i)
    {
        graphicsPixels[dest + destOffset + i] = c;
        rotoscopePixels[dest + destOffset + i] = ClearPixel;
    }
}

void GraphicsCopyLine(uint16_t sourceSeg, uint16_t destSeg, uint16_t si, uint16_t di, uint16_t count)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

    uint32_t src = (uint32_t)sourceSeg;
    uint32_t dest = (uint32_t)destSeg;

    src <<= 4; // Convert to linear addres
    src -= 0xa0000; // Subtract from EGA page
    src *= 4; // Convert to our SDL memory linear address

    dest <<= 4; // Convert to linear addres
    dest -= 0xa0000; // Subtract from EGA page
    dest *= 4; // Convert to our SDL memory linear address

    uint32_t srcOffset = (uint32_t)si * 4;
    uint32_t destOffset = (uint32_t)di * 4;

    for(uint32_t i = 0; i < count * 4; ++i)
    {
        graphicsPixels[dest + destOffset + i] = graphicsPixels[src + srcOffset + i];
        rotoscopePixels[dest + destOffset + i] = rotoscopePixels[src + srcOffset + i];
    }
}

uint8_t GraphicsPeek(int x, int y, uint32_t offset, Rotoscope* pc)
{
    auto pixel = GraphicsPeekDirect(x, y, offset, pc);

    auto it = std::find(std::begin(colortable), std::end(colortable), pixel);
    assert(it != std::end(colortable));
    return std::distance(std::begin(colortable), it);
}

uint32_t GraphicsPeekDirect(int x, int y, uint32_t offset, Rotoscope* pc)
{
    if(offset == 0)
    {
        assert(false);
        offset = 0xa000;
    }

    offset <<= 4; // Convert to linear addres
    offset -= 0xa0000; // Subtract from EGA page
    offset *= 4; // Convert to our SDL memory linear address

    y = 199 - y;

    if(x < 0 || x >= GRAPHICS_MODE_WIDTH || y < 0 || y >= GRAPHICS_MODE_HEIGHT)
    {
        return colortable[0];
    }

    if(pc)
    {
        *pc = rotoscopePixels[y * GRAPHICS_MODE_WIDTH + x + offset];
    }

    return graphicsPixels[y * GRAPHICS_MODE_WIDTH + x + offset];
}

void GraphicsPixelDirect(int x, int y, uint32_t color, uint32_t offset, Rotoscope pc)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

    if(offset == 0)
    {
        assert(false);
        offset = 0xa000;
    }

    offset <<= 4; // Convert to linear addres
    offset -= 0xa0000; // Subtract from EGA page
    offset *= 4; // Conver to our SDL memory linear address

    y = 199 - y;

    if(x < 0 || x >= GRAPHICS_MODE_WIDTH || y < 0 || y >= GRAPHICS_MODE_HEIGHT)
    {
        return;
    }

    if(pc.content == TextPixel)
    {
        printf("\n");
    }

    pc.argb = color;
    rotoscopePixels[y * GRAPHICS_MODE_WIDTH + x + offset] = pc;

    graphicsPixels[y * GRAPHICS_MODE_WIDTH + x + offset] = color;
}

void GraphicsLine(int x1, int y1, int x2, int y2, int color, int xormode, uint32_t offset)
{
    float x = x1;
    float y = y1;
    float dx = (x2 - x1);
    float dy = (y2 - y1);
    int n = fabs(dx);
    if (fabs(dy) > n) n = fabs(dy);
    if (n == 0) return;
    dx /= n;
    dy /= n;

    Rotoscope rs{};
    rs.content = LinePixel;
    rs.lineData.x0 = x1;
    rs.lineData.x1 = x2;
    rs.lineData.y0 = 199 - y1;
    rs.lineData.y1 = 199 - y2;
    rs.lineData.total = n;
    rs.lineData.fgColor = color;

    for(int i=0; i<=n; i++)
    {
        rs.lineData.n = i;

        rs.lineData.bgColor = GraphicsPeek(x, y, offset);
        GraphicsPixel(x, y, color, offset, rs);
        x += dx;
        y += dy;
    }
}

void GraphicsPixel(int x, int y, int color, uint32_t offset, Rotoscope pc)
{
    pc.EGAcolor = color & 0xf;
    GraphicsPixelDirect(x, y, colortable[color&0xF], offset, pc);
}

std::unordered_map<char, int> font1_table = {
    {' ', 0x0000}, {'!', 0x4904}, {'"', 0xB400}, {'#', 0xFFFF},
    {'$', 0xF45E}, {'%', 0xA54A}, {'&', 0x0000}, {'\'', 0x4800},
    {'[', 0x2922}, {']', 0x8928}, {'*', 0x1550}, {'+', 0x0BA0},
    {',', 0x0128}, {'-', 0x0380}, {'.', 0x0004}, {'/', 0x2548},
    {'0', 0xF6DE}, {'1', 0x4924}, {'2', 0xE7CE}, {'3', 0xE59E},
    {'4', 0xB792}, {'5', 0xF39E}, {'6', 0xD3DE}, {'7', 0xE524},
    {'8', 0xF7DE}, {'9', 0xF792}, {':', 0x0820}, {';', 0x0828},
    {'<', 0x2A22}, {'=', 0x1C70}, {'>', 0x88A8}, {'?', 0xE584},
    {'@', 0xFFCE}, {'A', 0x57DA}, {'B', 0xD75C}, {'C', 0x7246},
    {'D', 0xD6DC}, {'E', 0xF34E}, {'F', 0xF348}, {'G', 0x7256},
    {'H', 0xB7DA}, {'I', 0xE92E}, {'J', 0x24DE}, {'K', 0xB75A},
    {'L', 0x924E}, {'M', 0xBFDA}, {'N', 0xBFFA}, {'O', 0x56D4},
    {'P', 0xF7C8}, {'Q', 0xF7A6}, {'R', 0xF7EA}, {'S', 0x739C},
    {'T', 0xE924}, {'U', 0xB6DE}, {'V', 0xB6D4}, {'W', 0xB7FA},
    {'X', 0xB55A}, {'Y', 0xB7A4}, {'Z', 0xE54E}
};

std::unordered_map<char, std::array<uint16_t, 3>> font2_table = {
    {' ', {0x0000, 0x0000, 0x0000}}, {'!', {0x0000, 0x0000, 0x0000}},
    {'"', {0x0000, 0x0000, 0x0000}}, {'#', {0x0000, 0x0000, 0x0000}},
    {'$', {0x0000, 0x0000, 0x0000}}, {'%', {0x0000, 0x0000, 0x0000}},
    {'&', {0x0000, 0x0000, 0x0000}}, {'\'', {0x6F00, 0x0000, 0x0000}},
    {'[', {0x0000, 0x0000, 0x0000}}, {']', {0x0000, 0x0000, 0x0000}},
    {'*', {0x0000, 0x0000, 0x0000}}, {'+', {0x0000, 0x0000, 0x0000}},
    {',', {0x0006, 0xF000, 0x0000}}, {'-', {0x0070, 0x0000, 0x0000}},
    {'.', {0x0200, 0x0000, 0x0000}}, {'/', {0x0000, 0x0000, 0x0000}},
    {'0', {0x76F7, 0xBDED, 0xC000}}, {'1', {0x6718, 0xC633, 0xC000}},
    {'2', {0x76C6, 0x6663, 0xE000}}, {'3', {0x76C6, 0x61ED, 0xC000}},
    {'4', {0x35AD, 0x6F98, 0xC000}}, {'5', {0xFE31, 0xE1ED, 0xC000}},
    {'6', {0x76F1, 0xEDED, 0xC000}}, {'7', {0xFEC6, 0x6631, 0x8000}},
    {'8', {0x76F6, 0xEDED, 0xC000}}, {'9', {0x76F6, 0xF1ED, 0xC000}},
    {':', {0x1400, 0x0000, 0x0000}}, {';', {0x0000, 0x0000, 0x0000}},
    {'<', {0x0000, 0x0000, 0x0000}}, {'=', {0x0000, 0x0000, 0x0000}},
    {'>', {0x0000, 0x0000, 0x0000}}, {'?', {0x0000, 0x0000, 0x0000}},
    {'@', {0x0000, 0x0000, 0x0000}}, {'A', {0x3673, 0x9FE7, 0x2000}},
    {'B', {0xEDDE, 0xDDE7, 0x0000}}, {'C', {0x34CC, 0xC430, 0x0000}},
    {'D', {0xEDDD, 0xDDE0, 0x0000}}, {'E', {0xFCCE, 0xCCF0, 0x0000}},
    {'F', {0xFCCE, 0xCCC0, 0x0000}}, {'G', {0x3231, 0x8DA4, 0xC000}},
    {'H', {0xCE73, 0xFCE7, 0x2000}}, {'I', {0xF666, 0x66F0, 0x0000}},
    {'J', {0x3333, 0x3BF0, 0x0000}}, {'K', {0xDDDD, 0xEDD0, 0x0000}},
    {'L', {0xCCCC, 0xCCF0, 0x0000}}, {'M', {0xC71E, 0xF5C7, 0x1C40}},
    {'N', {0xCE7B, 0xBCE7, 0x3900}}, {'O', {0x3273, 0x9CE5, 0xC000}},
    {'P', {0xFDDD, 0xFCC0, 0x0000}}, {'Q', {0x312C, 0xB2D9, 0x2340}},
    {'R', {0xFDDD, 0xEDD0, 0x0000}}, {'S', {0x7CC6, 0x33E0, 0x0000}},
    {'T', {0xF666, 0x6660, 0x0000}}, {'U', {0xCE73, 0x9CE7, 0xE000}},
    {'V', {0xCE73, 0x9CE4, 0xC000}}, {'W', {0xC71C, 0x71D7, 0xF280}},
    {'X', {0xDDD2, 0xDDD0, 0x0000}}, {'Y', {0xDDDD, 0xF660, 0x0000}},
    {'Z', {0xF324, 0xCCF0, 0x0000}}
};

#include <unordered_map>

std::unordered_map<char, std::array<int, 4>> font3_table = {
    {'A', {0x3673, 0x9CFF, 0x39C8, 0x0000}},
    {'B', {0xEDDD, 0xEDDD, 0xE000, 0x0000}},
    {'C', {0x34CC, 0xCCC4, 0x3000, 0x0000}},
    {'D', {0xEDDD, 0xDDDD, 0xE000, 0x0000}},
    {'E', {0xFCCC, 0xECCC, 0xF000, 0x0000}},
    {'F', {0xFCCC, 0xECCC, 0xC000, 0x0000}},
    {'G', {0x3231, 0x8C6F, 0x2930, 0x0000}},
    {'H', {0xCE73, 0x9FE7, 0x39C8, 0x0000}},
    {'I', {0xF666, 0x6666, 0xF000, 0x0000}},
    {'J', {0x3333, 0x333B, 0xF000, 0x0000}},
    {'K', {0xDDDD, 0xDEDD, 0xD000, 0x0000}},
    {'L', {0xCCCC, 0xCCCC, 0xF000, 0x0000}},
    {'M', {0xC71E, 0xF5C7, 0x1C71, 0xC400}},
    {'N', {0xCE7B, 0xBCE7, 0x39C8, 0x0000}},
    {'O', {0x3273, 0x9CE7, 0x2930, 0x0000}},
    {'P', {0xFDDD, 0xDFCC, 0xC000, 0x0000}},
    {'Q', {0x312C, 0xB2CB, 0x2D92, 0x3400}},
    {'R', {0xFDDD, 0xDFED, 0xD000, 0x0000}},
    {'S', {0x7CCC, 0x6333, 0xE000, 0x0000}},
    {'T', {0xF666, 0x6666, 0x6000, 0x0000}},
    {'U', {0xCE73, 0x9CE7, 0x39F8, 0x0000}},
    {'V', {0xCE73, 0x9CE7, 0x3930, 0x0000}},
    {'W', {0xC71C, 0x71C7, 0x1D7A, 0x2800}},
    {'X', {0xDDDD, 0x2DDD, 0xD000, 0x0000}},
    {'Y', {0xDDDD, 0xDF66, 0x6000, 0x0000}},
    {'Z', {0xF332, 0x4CCC, 0xF000, 0x0000}}
};

std::unordered_map<char, int> char_width_table = {
    {' ', 4}, {'!', 4}, {'"', 4}, {'#', 4},
    {'$', 4}, {'%', 4}, {'&', 5}, {'\'', 3},
    {'[', 4}, {']', 4}, {'*', 4}, {'+', 4},
    {',', 3}, {'-', 3}, {'.', 1}, {'/', 4},
    {'0', 5}, {'1', 5}, {'2', 5}, {'3', 5},
    {'4', 5}, {'5', 5}, {'6', 5}, {'7', 5},
    {'8', 5}, {'9', 5}, {':', 1}, {';', 1},
    {'<', 4}, {'=', 4}, {'>', 4}, {'?', 4},
    {'@', 4}, {'A', 5}, {'B', 4}, {'C', 4},
    {'D', 4}, {'E', 4}, {'F', 4}, {'G', 5},
    {'H', 5}, {'I', 4}, {'J', 4}, {'K', 4},
    {'L', 4}, {'M', 6}, {'N', 5}, {'O', 5},
    {'P', 4}, {'Q', 6}, {'R', 4}, {'S', 4},
    {'T', 4}, {'U', 5}, {'V', 5}, {'W', 6},
    {'X', 4}, {'Y', 4}, {'Z', 4}
};

int16_t GraphicsFONT(uint16_t num, uint32_t character, int x1, int y1, int color, int xormode, uint32_t offset)
{
    char c = (char)character;

    Rotoscope rs{};

    rs.content = TextPixel;
    rs.textData.character = c;
    rs.textData.fontNum = num;
    rs.textData.fgColor = color;
    rs.textData.xormode = xormode;

    switch(num)
    {
        case 1:
        {
            auto width = 3;
            auto image = font1_table[c];
            rs.textData.fontWidth = width;
            rs.textData.fontHeight = 5;

            GraphicsBLT(x1, y1, 5, width, (const char*)&image, color, xormode, offset, rs);

            return width;
        }
        case 2:
        {
            auto width = char_width_table[c];
            auto image = font2_table[c].data();
            rs.textData.fontWidth = width;
            rs.textData.fontHeight = 7;

            GraphicsBLT(x1, y1, 7, width, (const char*)image, color, xormode, offset, rs);

            return width;
        }
        case 3:
        {
            auto width = char_width_table[c];
            auto image = font3_table[c].data();
            rs.textData.fontWidth = width;
            rs.textData.fontHeight = 9;

            GraphicsBLT(x1, y1, 9, width, (const char*)image, color, xormode, offset, rs);

            return width;            
        }
        default:
            assert(false);
            break;
    }

    assert(false);
    return 1;
}

void GraphicsBLT(int16_t x1, int16_t y1, int16_t h, int16_t w, const char* image, int color, int xormode, uint32_t offset, Rotoscope pc)
{
    auto img = (const short int*)image;
    int n = 0;

    uint16_t xoffset = 0;
    uint16_t yoffset = 0;

    pc.blt_w = w;
    pc.blt_h = h;

    for(int y=y1; y>y1-h; y--)
    {
        xoffset = 0;

        for(int x=x1; x<x1+w; x++)
        {
            int x0 = x;
            int y0 = y;


            Rotoscope srcPc{};
            bool hasPixel = false;
            auto src = GraphicsPeek(x0, y0, offset, &srcPc);

            pc.blt_x = xoffset;
            pc.blt_y = yoffset;

            if(pc.content == TextPixel)
            {
                pc.textData.bgColor = src;
            }

            if ((*img) & (1<<(15-n)))
            {
                if(xormode) {
                    auto xored = src ^ (color&0xF);

                    if(srcPc.content == TextPixel)
                    {
                        srcPc.textData.bgColor = srcPc.textData.bgColor ^ (color & 0xf);
                        srcPc.textData.fgColor = srcPc.textData.fgColor ^ (color & 0xf);
                        GraphicsPixel(x0, y0, xored, offset, srcPc);
                    }
                    else
                    {
                        GraphicsPixel(x0, y0, xored, offset, pc);
                    }
                }
                else
                {
                    GraphicsPixel(x0, y0, color, offset, pc);
                }
            }
            else
            {
                GraphicsPixel(x0, y0, src, offset, pc);
            }
            
            n++;
            if (n == 16)
            {
                n = 0;
                img++;
            }

            ++xoffset;
        }

        ++yoffset;
    }
}

bool GraphicsHasKey()
{
    return keyboard->checkForKeyStroke();
}

uint16_t GraphicsGetKey()
{
    return keyboard->getKeyStroke();
}

void GraphicsSave(char *filename)
{
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
      fprintf(stderr, "Error: Cannot write file\n");
      exit(1);
  }
  printf("Store image %s\n", filename);

  fprintf(file, "P3\n");
  fprintf(file, "%i %i\n", GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
  fprintf(file, "255\n");

  for(int j=0; j<GRAPHICS_MODE_HEIGHT; j++)
  {
      for(int i=0; i<GRAPHICS_MODE_WIDTH; i++)
      {
          int c = graphicsPixels[j * GRAPHICS_MODE_WIDTH + i];
          fprintf(file, "%i %i %i ", (c>>16)&0xFF, (c>>8)&0xFF, (c>>0)&0xFF);
      }
      fprintf(file, "\n");
  }
  fclose(file);
}

