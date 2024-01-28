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

#include <zstd.h>
#include <xxhash.h>

#include "../cpu/cpu.h"
#include "../tts/speech.h"

#include "vulkan_helper.h"

#if defined(PLANES)
    #undef PLANES
#endif

std::promise<void> initPromise;
std::future<void> initFuture = initPromise.get_future();

//#define USE_CPU_RASTERIZATION 1

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

#define FREQUENCY 44100 // Samples per second

static double toneInHz = 440.0;

static std::atomic<bool> s_useRotoscope = true;
static std::atomic<bool> s_useEGA = true;
static std::atomic<uint32_t> s_alienVar1 = 0;

static std::mutex s_deadReckoningMutex;
static vec2<int16_t> s_deadReckoning = {};
static vec2<int16_t> s_pastWorld = {};
static std::chrono::time_point<std::chrono::system_clock> s_deadReckoningSet;

static std::mutex s_frameCountMutex;
static uint64_t s_frameCount;
static uint64_t s_frameAccelCount;

struct GraphicsContext
{
    VulkanContext vc;
    avk::queue* mQueue;

    struct BufferData {
        avk::buffer frameStaging;
        avk::buffer rotoscope;
        avk::buffer uniform;
        avk::buffer iconUniform;
        avk::command_buffer command;
        avk::image_sampler navigation;
    };
    
    std::vector<BufferData> buffers;

    avk::image_sampler LOGO1;
    avk::image_sampler LOGO2;
    avk::image_sampler FONT1;
    avk::image_sampler FONT2;
    avk::image_sampler FONT3;

    avk::image_sampler PORTPIC;

    avk::image_sampler RACEDOSATLAS;

    avk::image_sampler textImage;

    avk::image_sampler shipImage;
    avk::image_sampler planetAlbedoImages;

    avk::image_sampler alienColorImage;
    avk::image_sampler alienDepthImage;
    avk::image_sampler alienBackgroundImage;

    avk::compute_pipeline rotoscopePipeline;
    avk::compute_pipeline navigationPipeline;
    avk::compute_pipeline textPipeline;

    avk::descriptor_cache descriptorCache;

    std::chrono::time_point<std::chrono::system_clock> epoch;

    std::unordered_map<uint32_t, PlanetSurface> surfaceData{};

    std::unordered_map<uint32_t, uint32_t> seedToIndex;

    bool shouldInitPlanets = false;
    std::binary_semaphore planetsDone{0};
};

static GraphicsContext s_gc{};

template<std::size_t PLANES>
union TextureColor {
    std::array<float, PLANES> u;

    constexpr float& r() { return u[0]; }
    constexpr float& g() { return u[1]; }
    constexpr float& b() { return u[2]; }
    constexpr float& a() { return u[3]; }

    constexpr const float& r() const { return u[0]; }
    constexpr const float& g() const { return u[1]; }
    constexpr const float& b() const { return u[2]; }
    constexpr const float& a() const { return u[3]; }    
};

template<std::size_t PLANES>
uint32_t TextureColorToARGB(const TextureColor<PLANES>& color) {
    uint32_t a = static_cast<uint32_t>(color.a() * 255.0f);
    uint32_t r = static_cast<uint32_t>(color.r() * 255.0f);
    uint32_t g = static_cast<uint32_t>(color.g() * 255.0f);
    uint32_t b = static_cast<uint32_t>(color.b() * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
struct Texture {
    std::array<std::array<TextureColor<PLANES>, WIDTH>, HEIGHT> data;
};

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
TextureColor<PLANES> bilinearSample(const Texture<WIDTH, HEIGHT, PLANES>& texture, float u, float v) {
    u *= WIDTH - 1;
    v *= HEIGHT - 1;

    int x = (int)u;
    int y = (int)v;
    float u_ratio = u - x;
    float v_ratio = v - y;
    float u_opposite = 1 - u_ratio;
    float v_opposite = 1 - v_ratio;

    TextureColor<PLANES> result;
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

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
void fillTextureLiteral(Texture<WIDTH, HEIGHT, PLANES>& texture, const std::vector<float>& image) {
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            for (int k = 0; k < PLANES; ++k) {
                texture.data[j][i].u[k] = image[(j * WIDTH + i) * PLANES + k];
            }
        }
    }
}

Texture<448, 160, 1> FONT1Texture;
Texture<840, 180, 1> FONT2Texture;
Texture<840, 220, 1> FONT3Texture;
Texture<2160, 392, 1> RaceDosPicTexture;

enum SFGraphicsMode
{
    Unset = -1,
    Text = 0,
    Graphics = 1,
};

SFGraphicsMode toSetGraphicsMode = Unset;
SFGraphicsMode graphicsMode = Unset;
std::counting_semaphore<1024> modeChangeComplete{ 0 };

FrameSync frameSync{};

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

bool graphicsIsShutdown = false;

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

static uint8_t s_keyboardState[1024] = {};

class DOSKeyboard {
public:
    // Destructive read equivalent to Int 16 ah = 0
    virtual bool checkForKeyStroke() = 0;
    // Destructive read equivalent to Int 16 ah = 0
    virtual unsigned short getKeyStroke() = 0;

    virtual void update() = 0;

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

    void update() override {

    }
};

class SDLKeyboard : public DOSKeyboard {
private:
    std::mutex eventQueueMutex;
    std::deque<SDL_Event> eventQueue{};
    std::counting_semaphore<1024> availableKeys{0};

    void pushEvent(const SDL_Event& event) {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        eventQueue.push_back(event);
        availableKeys.release(); // Increment the count of available keys
    }

    SDL_Event popEvent() {
        availableKeys.acquire(); // Wait until a key is available
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        SDL_Event event = eventQueue.front();
        eventQueue.pop_front();
        return event;
    }

    bool keysAvailable() {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        return !eventQueue.empty();
    }

    static bool isArrowOrKeypadKey(const SDL_Event& event) {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
            return false;
        }

        switch (event.key.keysym.sym) {
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_KP_8:
            case SDLK_KP_2:
            case SDLK_KP_4:
            case SDLK_KP_6:
            case SDLK_KP_7:
            case SDLK_KP_9:
            case SDLK_KP_1:
            case SDLK_KP_3:
                return true;
            default:
                return false;
        }
    }

    bool areArrowKeysDown() {
        const Uint8* state = (const Uint8*)s_keyboardState;
        return state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_RIGHT] ||
               state[SDL_SCANCODE_KP_8] || state[SDL_SCANCODE_KP_2] || state[SDL_SCANCODE_KP_4] || state[SDL_SCANCODE_KP_6] ||
               state[SDL_SCANCODE_KP_7] || state[SDL_SCANCODE_KP_9] || state[SDL_SCANCODE_KP_1] || state[SDL_SCANCODE_KP_3];
    }

    unsigned short getArrowKeyDown() {
        const Uint8* state = (const Uint8*)s_keyboardState;
        if (state[SDL_SCANCODE_UP]) {
            return 328;
        } else if (state[SDL_SCANCODE_DOWN]) {
            return 336;
        } else if (state[SDL_SCANCODE_LEFT]) {
            return 331;
        } else if (state[SDL_SCANCODE_RIGHT]) {
            return 333;
        } else if (state[SDL_SCANCODE_KP_8]) {
            return 328; // Numpad 8
        } else if (state[SDL_SCANCODE_KP_2]) {
            return 336; // Numpad 2
        } else if (state[SDL_SCANCODE_KP_4]) {
            return 331; // Numpad 4
        } else if (state[SDL_SCANCODE_KP_6]) {
            return 333; // Numpad 6
        } else if (state[SDL_SCANCODE_KP_7]) {
            return 327; // Numpad 7
        } else if (state[SDL_SCANCODE_KP_9]) {
            return 329; // Numpad 9
        } else if (state[SDL_SCANCODE_KP_1]) {
            return 335; // Numpad 1
        } else if (state[SDL_SCANCODE_KP_3]) {
            return 337; // Numpad 3
        } else {
            return 0; // No arrow key down
        }
    }

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

public:
    SDLKeyboard() {}    

    void update() override {

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
                    else
                    {
                        if (!isArrowOrKeypadKey(event))
                        {
                            pushEvent(event);
                        }
                    }
                    return true;
                    break;
                case SDL_KEYUP:
                    {
                        std::lock_guard<std::mutex> lg(s_deadReckoningMutex);
                        s_deadReckoning = { 0 , 0 };
                    }
                    break;
                case SDL_WINDOWEVENT:
                    {
                        if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        {
                            if(SDL_GetWindowFromID(event.window.windowID) == window)
                            {
                                GraphicsQuit();
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
                    break;
                default:
                    break;
            }

            return false;
        };

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            handleEvent(event);
        }
    }

    // Non-destructive read equivalent to Int 16 ah = 1
    bool checkForKeyStroke() override {
        if(areArrowKeysDown())
        {
            return true;
        }
       
        return keysAvailable();
    }

    // Destructive read equivalent to Int 16 ah = 0
    unsigned short getKeyStroke() override {
        auto arrow = getArrowKeyDown();
        if(arrow != 0)
        {
            return arrow;
        }

        SDL_Event event = popEvent();
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

std::mutex audioSource1Mutex;
std::deque<Sint16> audioSource1;

// All speech is single track 22050 hz mono.
void queue_speech(int16_t* voiceAudio, uint64_t length)
{
    std::lock_guard<std::mutex> lock1(audioSource1Mutex);

    for(int i = 0; i < length; ++i)
    {
        // Double the playback rate as the playback rate is 44100 hz
        audioSource1.push_back((Sint16)voiceAudio[i]);
        audioSource1.push_back((Sint16)voiceAudio[i]);
    }
}

// This is the function that gets automatically called every time the audio device needs more data
void play_buffer(void* userdata, unsigned char* stream, int len) {
	SDL_memset(stream, spec.silence, len);

    static unsigned long time = 0;
    Sint16 *stream16 = (Sint16*)stream;

    std::lock_guard<std::mutex> lock1(audioSource1Mutex);

    int64_t alienSound = 0;

    for(int i = 0; i < len/2; i++, time++) {
        Sint16 outSample = 0;
        Sint16 squareWave = 0;
        Sint16 speech = 0;

        if(s_audioPlaying) {
            squareWave = (Sint16)(square(toneInHz, time) * 2000.0);
        }

        if (!audioSource1.empty()) {
            speech = audioSource1.front();
            audioSource1.pop_front();
            alienSound += speech;
        }

        outSample = squareWave + speech;

        // Avoid overflow
        if (outSample > INT16_MAX) {
            outSample = INT16_MAX;
        } else if (outSample < INT16_MIN) {
            outSample = INT16_MIN;
        }

        if(outSample == 0)
        {
            outSample = spec.silence;
        }

        stream16[i] = (Sint16)outSample;
    }

    if (len > 1)
    {
        alienSound /= (len / 2);
        s_alienVar1 = (float)s_alienVar1 * 0.9f + abs(float(alienSound)) * 0.1f;
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

void GraphicsSetDeadReckoning(int16_t x, int16_t y)
{
    std::lock_guard<std::mutex> lg(s_deadReckoningMutex);
    s_deadReckoning = { x , y };
    s_deadReckoningSet = std::chrono::system_clock::now();

    uint64_t framesDrawn = s_frameCount;
    s_frameCount = 0;

    printf("Drew %d frames between one game frame GraphicsReportGameFrame\n", framesDrawn);
}

void GraphicsReportGameFrame()
{

}

RotoscopeShader& RotoscopeShader::operator=(const Rotoscope& other) {
    content = other.content;
    EGAcolor = other.EGAcolor;
    argb = other.argb;
    blt_x = other.blt_x;
    blt_y = other.blt_y;
    blt_w = other.blt_w;
    blt_h = other.blt_h;
    bgColor = other.bgColor;
    fgColor = other.fgColor;

    switch(other.content)
    {
        case ClearPixel:
        case EllipsePixel:
        case BoxFillPixel:
        case PlotPixel:
        case PolyFillPixel:
        case TilePixel:
        case NavigationalPixel:
        case PicPixel:
            break;
        case TextPixel:
            textData = other.textData;
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

    return *this;
}

static Texture<1536, 1152, 4> LOGO1Texture;
static Texture<1536, 1152, 4> LOGO2Texture;
static Texture<1536, 1152, 4> PORTPICTexture;

avk::image imageFromData(const void* data, uint32_t width, uint32_t height, uint32_t bytesPerPixel, vk::Format format, avk::image_usage usage)
{
    uint32_t dataSize = width * height * bytesPerPixel;

    auto sb = s_gc.vc.create_buffer(
        AVK_STAGING_BUFFER_MEMORY_USAGE,
        vk::BufferUsageFlagBits::eTransferSrc,
        avk::generic_buffer_meta::create_from_size(dataSize)
    );

    auto image = s_gc.vc.create_image(width, height, format, 1, avk::memory_usage::device, usage);

    s_gc.vc.record_and_submit_with_fence({

        sb->fill(data, 0, 0, dataSize),

        avk::sync::buffer_memory_barrier(sb.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
            ),

        avk::sync::image_memory_barrier(image.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::undefined, avk::layout::transfer_dst}),

        avk::copy_buffer_to_image(sb, image, avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(image.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::transfer_dst, avk::layout::shader_read_only_optimal})        

    }, *s_gc.mQueue)->wait_until_signalled();

    return image;
}

void LoadSplashImages()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    struct ImageToLoad
    {
        std::string name;
        Texture<1536, 1152, 4>& pic;
        avk::image_sampler& vkPic;

        ImageToLoad(const std::string& name, Texture<1536, 1152, 4>& pic, avk::image_sampler& vkPic)
            : name(name), pic(pic), vkPic(vkPic) {}
    };

    static const std::vector<ImageToLoad> images = {
        { "logo_1.png", LOGO1Texture, s_gc.LOGO1 },
        { "logo_2.png", LOGO2Texture, s_gc.LOGO2 },
        { "station.png", PORTPICTexture, s_gc.PORTPIC }
    };

    for (auto& img : images)
    {
        unsigned error = lodepng::decode(image, width, height, img.name, LCT_RGBA, 8);
        if (error)
        {
            printf("decoder error %d, %s loading %s\n", error, lodepng_error_text(error), img.name.c_str());
            exit(-1);
        }

        fillTexture(img.pic, image);
        img.vkPic = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                imageFromData(image.data(), width, height, 4, vk::Format::eR8G8B8A8Unorm, avk::image_usage::general_image)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
        );
        image.clear();
    }
}

void LoadSDFImages()
{
    std::ifstream file("atlas.raw.zst", std::ios::binary | std::ios::ate);
    if (!file)
    {
        printf("Error opening atlas.raw.zst\n");
        exit(-1);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> compressedData(size);
    if (!file.read(compressedData.data(), size))
    {
        printf("Error reading atlas.raw.zst\n");
        exit(-1);
    }

    unsigned long long const rSize = ZSTD_getFrameContentSize(compressedData.data(), size);
    if (rSize == ZSTD_CONTENTSIZE_ERROR || rSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
        printf("Error determining decompressed size\n");
        exit(-1);
    }

    std::vector<float> buffer(rSize / sizeof(float));
    size_t const dSize = ZSTD_decompress(buffer.data(), rSize, compressedData.data(), size);
    if (ZSTD_isError(dSize))
    {
        printf("Error decompressing atlas.raw.zst: %s\n", ZSTD_getErrorName(dSize));
        exit(-1);
    }

    fillTextureLiteral(RaceDosPicTexture, buffer);

    auto image = imageFromData(buffer.data(), RaceDosPicTexture.data[0].size(), RaceDosPicTexture.data.size(), 4, vk::Format::eR32Sfloat, avk::image_usage::general_image);
    s_gc.RACEDOSATLAS = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(image), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
}

void LoadFonts()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, "FONT1_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT1Texture, image);
    s_gc.FONT1 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

    error = lodepng::decode(image, width, height, "FONT2_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT2Texture, image);
    s_gc.FONT2 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

    error = lodepng::decode(image, width, height, "FONT3_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT3Texture, image);
    s_gc.FONT3 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );    
    image.clear();
}

void LoadAssets()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, "ship.png", LCT_RGBA, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    s_gc.shipImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 4, vk::Format::eR8G8B8A8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::trilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

    s_gc.planetAlbedoImages = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(48, 24, vk::Format::eR8G8B8A8Unorm, 811, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienColorImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienBackgroundImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienDepthImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.vc.record_and_submit_with_fence({
            avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienColorImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienDepthImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienBackgroundImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

    }, *s_gc.mQueue)->wait_until_signalled();
}

void GraphicsInitPlanets(std::unordered_map<uint32_t, PlanetSurface> surfaces)
{
    s_gc.surfaceData = surfaces;

    s_gc.shouldInitPlanets = true;

    s_gc.planetsDone.acquire();
}

static int GraphicsInitThread()
{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

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
    
    s_gc.vc.vulkan_instance();
    s_gc.vc.physical_device();
    s_gc.vc.device();
    s_gc.vc.create_swap_chain(VulkanContext::swapchain_creation_mode::create_new_swapchain, window, WINDOW_WIDTH, WINDOW_HEIGHT);

    s_gc.mQueue = s_gc.vc.getAVKQueue();

    auto& commandPool = s_gc.vc.get_command_pool_for_resettable_command_buffers(*s_gc.mQueue);

    // Navigation window is 72 x 120 pixels.
    uint32_t navWidth = (uint32_t)ceilf((float(NagivationWindowWidth) / 160.0f) * (float)WINDOW_WIDTH);
    uint32_t navHeight = (uint32_t)ceilf((float(NagivationWindowHeight) / 200.0f) * (float)WINDOW_HEIGHT);

    for (int i = 0; i < s_gc.vc.number_of_frames_in_flight(); ++i)
    {
        GraphicsContext::BufferData bd{};

        bd.frameStaging = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferSrc,
                avk::generic_buffer_meta::create_from_size(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t)));

        bd.rotoscope = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
                avk::storage_buffer_meta::create_from_size(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT * sizeof(RotoscopeShader)));

        bd.uniform = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(UniformBlock)));

        bd.command = 
            commandPool->alloc_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        bd.navigation = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(navWidth, navHeight, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

        bd.iconUniform =
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(IconUniform)));

        s_gc.buffers.push_back(std::move(bd));
    }

/*
std::array<vk::DescriptorSetLayoutBinding, 8> bindings = {
    vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute), // imgOutput
    vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // RotoBuffer
    vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT1Texture
    vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT2Texture
    vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT3Texture
    vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // LOGO1Texture
    vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // LOGO2Texture
    vk::DescriptorSetLayoutBinding(7, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute) // uniforms
};
*/

    s_gc.rotoscopePipeline = s_gc.vc.create_compute_pipeline_for(
        "rotoscope.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 4, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 5, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 6, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 7, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 8, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 9, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 10, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 11, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 12, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 13, 1u),
        avk::descriptor_binding<avk::buffer>(0, 14, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 15, s_gc.buffers[0].iconUniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 16, 1u)
    );

    s_gc.navigationPipeline = s_gc.vc.create_compute_pipeline_for(
        "navigation.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].iconUniform)
    );

    s_gc.textPipeline = s_gc.vc.create_compute_pipeline_for(
        "text.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 1, 1u),
        avk::descriptor_binding<avk::buffer>(0, 2, s_gc.buffers[0].uniform)
    );

    s_gc.descriptorCache = s_gc.vc.create_descriptor_cache();

    LoadFonts();
    LoadSplashImages();
    LoadSDFImages();
    LoadAssets();

    s_gc.textImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(TEXT_MODE_WIDTH, TEXT_MODE_HEIGHT, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ),
        s_gc.vc.create_sampler(avk::filter_mode::nearest_neighbor, avk::border_handling_mode::clamp_to_edge)
    );

    s_gc.epoch = std::chrono::system_clock::now();

    keyboard = std::make_unique<SDLKeyboard>();

    graphicsPixels = std::vector<uint32_t>();
    graphicsPixels.resize(GRAPHICS_MEMORY_ALLOC);

    rotoscopePixels = std::vector<Rotoscope>();
    rotoscopePixels.resize(GRAPHICS_MEMORY_ALLOC);

    textPixels = std::vector<uint32_t>();
    textPixels.resize(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT);

    initPromise.set_value();

    return 0;
}

void GraphicsSplash(uint16_t seg, uint16_t fileNum)
{
    s_gc.epoch = std::chrono::system_clock::now();
}

void GraphicsInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }
    FILE* file;
    file = freopen("stdout", "w", stdout); // redirects stdout
    file = freopen("stderr", "w", stderr); // redirects stderr

    graphicsThread = std::jthread([] {
        GraphicsInitThread();

        while (!stopSemaphore.try_acquire()) {
            constexpr std::chrono::nanoseconds scanout_duration = std::chrono::nanoseconds(13340000); // 80% of 1/60th of a second
            constexpr std::chrono::nanoseconds retrace_duration = std::chrono::nanoseconds(3330000); // 20% of 1/60th of a second

            {
                std::lock_guard<std::mutex> lg(graphicsRetrace);
                GraphicsUpdate();
                if (graphicsIsShutdown)
                    return;

#if defined(USE_CPU_RASTERIZATION)
                std::this_thread::sleep_for(scanout_duration);
#endif
            }

#if defined(USE_CPU_RASTERIZATION)
            std::this_thread::sleep_for(retrace_duration);
#endif
        }
    });
}

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
        pixel = colortable[roto.fgColor & 0xf];
    }
    else
    {
        pixel = colortable[roto.bgColor & 0xf];
    }

    return pixel;
}

uint32_t DrawFontPixel(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv)
{
    uint32_t pixel = 0;

    float fontX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float fontY = ((float)roto.blt_y + subUv.v) / (float)roto.blt_h;

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
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.80f)
        {
            pixel = colortable[roto.fgColor & 0xf];
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

        auto glyph = bilinearSample(FONT2Texture, u, v);
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.9f)
        {
            pixel = colortable[roto.fgColor & 0xf];
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
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.9f)
        {
            pixel = colortable[roto.fgColor & 0xf];
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

uint32_t DrawSDFSilhouette(const Rotoscope& roto, vec2<float> subUv) {
    auto sdf = bilinearSample(RaceDosPicTexture, subUv.x, subUv.y);

    if (sdf.r() < 0.5)
    {
        return colortable[roto.bgColor & 0xf];
    }
    else
    {
        return colortable[roto.fgColor & 0xf];
    }
}

float calculateDistance(vec2<float> point1, vec2<float> point2) {
    float xDist = point2.x - point1.x;
    float yDist = 0.60f * (point2.y - point1.y);
    return sqrt((xDist * xDist) + (yDist * yDist));
}

float calculateBoundingBoxDistance(vec2<float> point1, vec2<float> point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = abs(point2.y - point1.y);
    return max(xDist, yDist);
}

float calculateBoundingBoxDistanceIcon(vec2<float> point1, vec2<float> point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = 0.60f * abs(point2.y - point1.y);
    return max(xDist, yDist);
}

uint32_t DrawNavigationPixel(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv, const std::vector<Icon>& icons)
{
    uint32_t pixel = 0;
    vec2<float> sub{};

    float subX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float subY = ((float)roto.blt_y + subUv.y) / (float)roto.blt_h;

    /* 
    50     C= NULL-ICON    ( 0 radius circle icon)
    253    C= SYS-ICON     ( star icon)                             
    254    C= INVIS-ICON   ( invisible icon - may collide)          
    255    C= FLUX-ICON    ( flux icon identifier)   
    */

/*
 @IX @IY ?INVIS               \ in the vis area?                
  @ID 51 91 WITHIN OR           \ or a filled circle            
  IF @ID NULL-ICON <                                            
    IF .8X8ICON ELSE @ID .ICONCASES THEN                        
  THEN ;                                                        
  */

    auto pixelPos = vec2<float>((float)roto.blt_x + subUv.x, (float)roto.blt_y + subUv.y);

    for(auto icon : icons)
    {
        auto iconPos = vec2<float>((float)icon.screenX, (float)icon.screenY);
        auto bltPos = vec2<float>((float)icon.bltX + 3.0f, (float)icon.bltY + 3.0f);

        if(icon.id >= 51 && icon.id <= 91)
        {
            float basesize = 29.0f * (float)(icon.id - 50);

            if(calculateDistance(iconPos, pixelPos) < basesize)
            {
                return colortable[icon.clr & 0xf];
            }
        }
        else if(icon.id < 50)
        {
            // Draw 8x8 icon. #29 is our spacecraft
            if(calculateBoundingBoxDistance(bltPos, pixelPos) < 4.0f)
            {
                return colortable[icon.clr & 0xf];
            }
        }
        else
        {
            float distance = 1.0;

            switch(icon.id)
            {
                case 253: // Star icon
                    /*
                    CASE SYSCASES ( color -- *bltseg-pfa )                          
                    RED    IS SSYSEG   \ small starsystem blt                     
                    ORANGE IS SSYSEG   \ medium starsystem blt                    
                    WHITE  IS MSYSEG   \ medium starsystem blt                    
                    YELLOW IS MSYSEG   \ medium starsystem blt                    
                    OTHERS LSYSEG        \ large starsystem blt  
                    */
                    
                    switch(icon.clr & 0xf)
                    {
                        case 0x4: // RED
                        case 0x6: // ORANGE
                            distance = 7.0f;
                            break;
                        case 0xE: // YELLOW
                        case 0xF: // WHITE
                            distance = 10.0f;
                            break;
                        default: // OTHERS
                            distance = 14.0f;
                            break;
                    }
                    if(calculateBoundingBoxDistanceIcon(iconPos, pixelPos) < distance)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                case 254: // Invisible icon (Encounter hit testing?)
                    if(roto.blt_x == icon.screenX && roto.blt_y == icon.screenY)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                case 255: // Flux icon
                    if(roto.blt_x == icon.screenX && roto.blt_y == icon.screenY)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }

    }
    
    return 0x0;
}

uint32_t DrawRunBit(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv)
{
    uint32_t pixel = 0;
    vec2<float> sub{};

    float subX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float subY = ((float)roto.blt_y + subUv.y) / (float)roto.blt_h;

    switch (roto.runBitData.tag)
    {
        case 44: // Port-Pic Top 100 pixels
            pixel = TextureColorToARGB(bilinearSample(PORTPICTexture, subX, subY * 0.5f));
            break;
        case 49: // Port-Pic Botton 100 pixels
            pixel = TextureColorToARGB(bilinearSample(PORTPICTexture, subX, (subY * 0.5f) + 0.5f));
            break;
        case 141: // First splash
            pixel = TextureColorToARGB(bilinearSample(LOGO1Texture, uv.u, uv.v));
            break;
        case 54:
            pixel = TextureColorToARGB(bilinearSample(LOGO2Texture, uv.u, uv.v));
            break;
        case 125:
            sub = vec2<float>(subX / 5.0f, subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 126:
            sub = vec2<float>(subX / 5.0f + (1.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 127:
            sub = vec2<float>(subX / 5.0f + (2.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 128:
            sub = vec2<float>(subX / 5.0f + (3.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 129:
            sub = vec2<float>(subX / 5.0f + (4.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        default:
            //assert(false);
            pixel = roto.argb;
            break;
    }

    return pixel;
}

void DoRotoscope(std::vector<uint32_t>& windowData, const std::vector<Rotoscope>& rotoPixels, const std::vector<Icon>& icons)
{
    uint32_t index = 0;
    const float polygonWidth = (float)WINDOW_WIDTH / (float)GRAPHICS_MODE_WIDTH;

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
                    case RunBitPixel:
                        pixel = DrawRunBit(roto, uv, subUv);
                        break;
                    case NavigationalPixel:
                        pixel = DrawNavigationPixel(roto, uv, subUv, icons);
                        break;
                    default:
                        //pixel = 0xffff0000;
                        pixel = colortable[(int)roto.content];
                        break;
                }
            }
            else
            {
                if(!s_useEGA)
                {
                    static uint32_t CGAPalette[4] = {0xff000000, 0xff00aaaa, 0xffaa00aa, 0xffaaaaaa};

                    uint8_t c = EGAToCGA[roto.EGAcolor];
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

std::vector<avk::recorded_commands_t> TextPass(VulkanContext::frame_id_t inFlightIndex, UniformBlock& uniform, const void* data, uint64_t dataSize)
{
    auto sb = s_gc.vc.create_buffer(
        AVK_STAGING_BUFFER_MEMORY_USAGE,
        vk::BufferUsageFlagBits::eTransferSrc,
        avk::generic_buffer_meta::create_from_size(dataSize)
    );

    return {
        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }),

        sb->fill(data,0, 0, dataSize),

        avk::sync::buffer_memory_barrier(sb.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::sync::image_memory_barrier(s_gc.textImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::transfer_dst }),

        avk::copy_buffer_to_image(sb, s_gc.textImage->get_image(), avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(s_gc.textImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }),

        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].uniform.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::command::bind_pipeline(s_gc.textPipeline.as_reference()),
        avk::command::bind_descriptors(s_gc.textPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, s_gc.vc.current_backbuffer_reference().image_view_at(0)->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.textImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 2, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer())
            })),
        avk::command::dispatch((WINDOW_WIDTH + 31u) / 32u, (WINDOW_HEIGHT + 31u) / 32u, 1),

        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::present_src })
    };
}

std::vector<avk::recorded_commands_t> CPUCopyPass(VulkanContext::frame_id_t inFlightIndex, const void* data, uint64_t dataSize)
{
    return {
        s_gc.buffers[inFlightIndex].frameStaging->fill(data, 0, 0, dataSize),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].frameStaging.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
            ),

        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::undefined, avk::layout::transfer_dst}),

        avk::copy_buffer_to_image(s_gc.buffers[inFlightIndex].frameStaging, s_gc.vc.current_backbuffer()->image_at(0), avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::transfer_dst, avk::layout::present_src})
    };
}

std::vector<avk::recorded_commands_t> GPURotoscope(VulkanContext::frame_id_t inFlightIndex, UniformBlock& uniform, IconUniform& iconUniform, const std::vector<RotoscopeShader>& shaderBackBuffer, bool hasNavigation)
{
    std::vector<avk::recorded_commands_t> res{};

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

    res.push_back(
        s_gc.buffers[inFlightIndex].rotoscope->fill(shaderBackBuffer.data(), 0, 0, shaderBackBuffer.size() * sizeof(RotoscopeShader)));

    res.push_back(
        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].rotoscope.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ));

    res.push_back(
        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)));

    res.push_back(
        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].rotoscope.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ));

    res.push_back(
        s_gc.buffers[inFlightIndex].iconUniform->fill(&iconUniform, 0, 0, sizeof(IconUniform)));

    if(hasNavigation)
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::bind_pipeline(s_gc.navigationPipeline.as_reference()));

        res.push_back(
            avk::command::bind_descriptors(s_gc.navigationPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                    avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].navigation->get_image_view()->as_storage_image(avk::layout::general)),
                    avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                    avk::descriptor_binding(0, 2, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 3, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 4, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                    avk::descriptor_binding(0, 5, s_gc.buffers[inFlightIndex].iconUniform->as_uniform_buffer())
                })));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));     

        uint32_t navWidth = (uint32_t)ceilf((float(NagivationWindowWidth) / 160.0f) * (float)WINDOW_WIDTH);
        uint32_t navHeight = (uint32_t)ceilf((float(NagivationWindowHeight) / 200.0f) * (float)WINDOW_HEIGHT);                

        res.push_back(
            avk::command::dispatch((navWidth + 3) / 4u, (navHeight + 3) / 4u, 1));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal }));                        
    }
    else
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }));
    }

    res.push_back(
        avk::command::bind_pipeline(s_gc.rotoscopePipeline.as_reference()));

    res.push_back(
        avk::command::bind_descriptors(s_gc.rotoscopePipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, s_gc.vc.current_backbuffer_reference().image_view_at(0)->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                avk::descriptor_binding(0, 2, s_gc.FONT1->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 3, s_gc.FONT2->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 4, s_gc.FONT3->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 5, s_gc.LOGO1->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 6, s_gc.LOGO2->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 7, s_gc.PORTPIC->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 8, s_gc.RACEDOSATLAS->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 9, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 10, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 11, s_gc.alienColorImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 12, s_gc.alienDepthImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 13, s_gc.alienBackgroundImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 14, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                avk::descriptor_binding(0, 15, s_gc.buffers[inFlightIndex].iconUniform->as_uniform_buffer()),
                avk::descriptor_binding(0, 16, s_gc.buffers[inFlightIndex].navigation->as_combined_image_sampler(avk::layout::shader_read_only_optimal))
            })));

    res.push_back(
        avk::command::dispatch((WINDOW_WIDTH + 3) / 4u, (WINDOW_HEIGHT + 3) / 4u, 1));

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::present_src }));

    return res;
}

uint32_t IconUniform::IndexFromSeed(uint32_t seed)
{
    if(seed == 0)
    {
        return 0;
    }

    assert(s_gc.surfaceData.size());

    auto it = s_gc.seedToIndex.find(seed);
    assert(it != s_gc.seedToIndex.end());

    return it->second;
}

void GraphicsUpdate()
{
    if (graphicsMode != toSetGraphicsMode)
    {
        graphicsMode = toSetGraphicsMode;

        std::fill(graphicsPixels.begin(), graphicsPixels.end(), 0);
        std::fill(textPixels.begin(), textPixels.end(), 0);
        std::fill(rotoscopePixels.begin(), rotoscopePixels.end(), ClearPixel);

        modeChangeComplete.release();
    }

    SDL_Texture* currentTexture = NULL;
    uint32_t stride = 0;
    const void* data = nullptr;
    size_t dataSize = 0;

    static std::vector<uint32_t> fullRes{};
    static std::vector<Rotoscope> backbuffer{};

    static std::vector<RotoscopeShader> shaderBackBuffer{};
    static UniformBlock uniform{};

    if (s_gc.shouldInitPlanets)
    {
        const uint32_t mapSize = 48 * 24 * 4;
        const uint32_t dataSize = mapSize * s_gc.surfaceData.size();

        auto sb = s_gc.vc.create_buffer(
            AVK_STAGING_BUFFER_MEMORY_USAGE,
            vk::BufferUsageFlagBits::eTransferSrc,
            avk::generic_buffer_meta::create_from_size(dataSize)
        );

        std::vector<avk::recorded_commands_t> commands{};

        commands.push_back(
            avk::sync::buffer_memory_barrier(sb.as_reference(),
                avk::stage::auto_stage >> avk::stage::auto_stage,
                avk::access::auto_access >> avk::access::auto_access
            ));

        commands.push_back(
            avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::shader_read_only_optimal, avk::layout::transfer_dst }));

        int i = 0;
        for(auto& ps : s_gc.surfaceData)
        {
            commands.push_back(sb->fill(ps.second.albedo.data(), 0, i * mapSize, mapSize));
            commands.push_back(avk::copy_buffer_to_image_layer_mip_level(sb, s_gc.planetAlbedoImages->get_image(), i, 0, avk::layout::transfer_dst, vk::ImageAspectFlagBits::eColor, i * mapSize));

            s_gc.seedToIndex.emplace(ps.first, i);

            ++i;
        }

        commands.push_back(avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));

        s_gc.vc.record_and_submit_with_fence(commands, *s_gc.mQueue)->wait_until_signalled();

        s_gc.shouldInitPlanets = false;
        s_gc.planetsDone.release();
    }

    uint32_t gameContext;
    std::vector<Icon> icons = GetLocalIconList(&gameContext);

    if (fullRes.size() == 0)
    {
        fullRes.resize(WINDOW_WIDTH * WINDOW_HEIGHT);
        backbuffer.resize(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT);
        shaderBackBuffer.resize(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT);

        uniform.graphics_mode_width = GRAPHICS_MODE_WIDTH;
        uniform.graphics_mode_height = GRAPHICS_MODE_HEIGHT;
        uniform.window_width = WINDOW_WIDTH;
        uniform.window_height = WINDOW_HEIGHT;
    }

    uniform.useEGA = s_useEGA ? 1 : 0;
    uniform.useRotoscope = s_useRotoscope ? 1 : 0;
    uniform.iTime = std::chrono::duration<float>(std::chrono::system_clock::now() - s_gc.epoch).count();
    uniform.game_context = gameContext;
    uniform.alienVar1 = s_alienVar1;

    bool hasNavigation = false;
    static uint32_t activeAlien = 0;

    std::vector<avk::recorded_commands_t> commands{};

    auto setActiveAlien = [&](uint32_t newAlien){
        const static std::unordered_map<uint32_t, std::string> aliens = {
            { 9 , "mechan"}
        };

        if(activeAlien != newAlien)
        {
            const uint32_t dataSize = 512 * 512 * 4;

            std::array<avk::image_sampler, 3> alienImgs = {
                s_gc.alienColorImage,
                s_gc.alienDepthImage,
                s_gc.alienBackgroundImage
            };

            std::array<std::string, 3> files = {
                "npc.png",
                "depth.png",
                "background.png"
            };

            for(int i = 0; i < 3; i++) {
                auto sb = s_gc.vc.create_buffer(
                    AVK_STAGING_BUFFER_MEMORY_USAGE,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    avk::generic_buffer_meta::create_from_size(dataSize)
                );

                std::vector<uint8_t> image;
                unsigned width, height;

                std::string filename = "mechan/" + files[i];

                unsigned error = lodepng::decode(image, width, height, filename, LCT_RGBA, 8);
                if (error)
                {
                    printf("decoder error %d, %s loading %s\n", error, lodepng_error_text(error), filename.c_str());
                    exit(-1);
                }

                commands.push_back(
                    avk::sync::image_memory_barrier(alienImgs[i]->get_image(),
                        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::shader_read_only_optimal, avk::layout::transfer_dst }));

                sb->fill(image.data(), 0, 0, dataSize),

                commands.push_back(
                    avk::sync::buffer_memory_barrier(sb.as_reference(),
                        avk::stage::auto_stage >> avk::stage::auto_stage,
                        avk::access::auto_access >> avk::access::auto_access
                    ));

                commands.push_back(avk::copy_buffer_to_image(sb, alienImgs[i]->get_image(), avk::layout::transfer_dst));

                commands.push_back(
                    avk::sync::image_memory_barrier(alienImgs[i]->get_image(),
                        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));

            }
            
            activeAlien = newAlien;
        }
    };

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

            for (int i = 0; i < GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT; ++i)
            {
#if defined(USE_CPU_RASTERIZATION)
                backbuffer[i] = rotoscopePixels[i];
#else
                shaderBackBuffer[i] = rotoscopePixels[i];

                if (rotoscopePixels[i].content == NavigationalPixel)
                {
                    hasNavigation = true;
                }

                if(rotoscopePixels[i].content == RunBitPixel)
                {
                    switch(rotoscopePixels[i].runBitData.tag)
                    {
                        case 9: // MECHAN 9
                            setActiveAlien(rotoscopePixels[i].runBitData.tag);
                            break;
                        default:
                        break;
                    }
                }
#endif
            }
        }

#if 0
        static int frameCount = 0;
        std::string filename = "frames/frame_" + std::to_string(frameCount++) + ".png";
        std::vector<unsigned char> png;
        unsigned width = GRAPHICS_MODE_WIDTH, height = GRAPHICS_MODE_HEIGHT;
        std::vector<unsigned char> image;
        image.resize(width * height * 4);
        for (unsigned y = 0; y < height; y++)
        {
            for (unsigned x = 0; x < width; x++)
            {
                uint32_t pixel = graphicsPixels[y * width + x];
                image[4 * width * y + 4 * x + 0] = (pixel >> 16) & 0xFF; // R
                image[4 * width * y + 4 * x + 1] = (pixel >> 8) & 0xFF; // G
                image[4 * width * y + 4 * x + 2] = pixel & 0xFF; // B
                image[4 * width * y + 4 * x + 3] = 255; // A
            }
        }
        unsigned error = lodepng::encode(filename, image, width, height);
        if (error)
        {
            printf("encoder error %u: %s\n", error, lodepng_error_text(error));
        }
#endif    

#if defined(USE_CPU_RASTERIZATION)
        DoRotoscope(fullRes, backbuffer, icons);
#endif
        currentTexture = windowTexture;
        stride = WINDOW_WIDTH;
        data = fullRes.data();
        dataSize = fullRes.size() * sizeof(uint32_t);
#endif
    }
    else if (graphicsMode == SFGraphicsMode::Text)
    {
        currentTexture = textTexture;
        stride = TEXT_MODE_WIDTH;
        data = textPixels.data();
        dataSize = textPixels.size() * sizeof(uint32_t);
    }

    s_gc.vc.sync_before_render();

    auto imageAvailableSemaphore = s_gc.vc.consume_current_image_available_semaphore();
    const auto inFlightIndex = s_gc.vc.in_flight_index_for_frame();

    s_gc.buffers[inFlightIndex].command->reset();

    if (graphicsMode == SFGraphicsMode::Text)
    {
        auto textCommands = TextPass(inFlightIndex, uniform, data, dataSize);
        commands.insert(commands.end(), textCommands.begin(), textCommands.end());
    }
    else
    {
#if defined(USE_CPU_RASTERIZATION)
        auto cpuCommands = CPUCopyPass(inFlightIndex, data, dataSize);
        commands.insert(commands.end(), cpuCommands.begin(), cpuCommands.end());
#else

        IconUniform ic(icons);
        auto gpuCommands = GPURotoscope(inFlightIndex, uniform, ic, shaderBackBuffer, hasNavigation);
        commands.insert(commands.end(), gpuCommands.begin(), gpuCommands.end());
#endif
    }

    if (hasNavigation)
    {
        int16_t worldCoordsX = (int16_t)Read16(0x5dae);
        int16_t worldCoordsY = (int16_t)Read16(0x5db9);
        int16_t heading = (int16_t)Read16(0x5dc7);

        uniform.heading = (float)heading;

        std::lock_guard<std::mutex> lg(s_deadReckoningMutex);

        // Will figure out navigation smooth scrolling eventually
        auto deadSet = std::chrono::duration<float>(std::chrono::system_clock::now() - s_deadReckoningSet).count();
        uniform.deadX = (float)s_deadReckoning.x * ((float)s_frameCount / 4.0f);
        uniform.deadY = (float)s_deadReckoning.y * ((float)s_frameCount / 4.0f);

        uniform.worldX = (float)worldCoordsX;
        uniform.worldY = (float)worldCoordsY;

        if (s_pastWorld != vec2<int16_t>(worldCoordsX, worldCoordsY))
        {
            s_frameAccelCount = 0;
        }

        int16_t cursorX = (int16_t)Read16(0xd9f6);
        int16_t cursorY = (int16_t)Read16(0xd9fa);
        int16_t espeed = (int16_t)Read16(0xda0a);
        int16_t acc = (int16_t)Read16(0xe921);

        printf("Frame: x %d y %d, crs x %d, y %d, espeed %d acc %d deadx %f deady %f xd %f yd %f\n", worldCoordsX, worldCoordsY, cursorX, cursorY, espeed, acc, 
            uniform.deadX, uniform.deadY,
            uniform.worldX,
            uniform.worldY);

        s_pastWorld = vec2<int16_t>(worldCoordsX, worldCoordsY);

        ++s_frameAccelCount;
    }

    s_gc.vc.record(std::move(commands))
    .into_command_buffer(s_gc.buffers[inFlightIndex].command)
    .then_submit_to(*s_gc.mQueue)
    // Do not start to render before the image has become available:
    .waiting_for(imageAvailableSemaphore >> avk::stage::color_attachment_output)
    .submit();

    {
        std::lock_guard<std::mutex> lg(s_deadReckoningMutex);
        ++s_frameCount;
    }

    s_gc.vc.render_frame();

    int numkeys = 0;
    auto keys = SDL_GetKeyboardState(&numkeys);
    assert(numkeys < sizeof(s_keyboardState));
    memcpy(s_keyboardState, keys, numkeys);

    SDL_PumpEvents();
    keyboard->update();
    if (graphicsIsShutdown)
        return;


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

    {
        std::lock_guard<std::mutex> lock(frameSync.mutex);
        frameSync.completedFrames++;
    }
    frameSync.frameCompleted.notify_one();
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
    if (graphicsIsShutdown)
        return;

    StopSpeech();

    if(std::this_thread::get_id() != graphicsThread.get_id())
    {
        if(graphicsThread.joinable())
        {
            stopSemaphore.release();

            s_gc.vc.device().waitIdle();

            graphicsThread.join();
        }
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

    graphicsIsShutdown = true;
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
    if (graphicsMode != Text)
    {   
        //assert(false);
        return;
    }
    
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
    if (toSetGraphicsMode == mode)
        return;

    toSetGraphicsMode = (SFGraphicsMode)mode;

    modeChangeComplete.acquire();
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

    if (pc.content == ClearPixel)
    {
        //assert(color == 0);
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
    rs.fgColor = color;

    for(int i=0; i<=n; i++)
    {
        rs.lineData.n = i;

        rs.bgColor = GraphicsPeek(x, y, offset);
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
    rs.fgColor = color;
    rs.textData.xormode = xormode;

    switch(num)
    {
        case 1:
        {
            auto width = 3;
            auto height = 5;
            auto image = font1_table[c];

            GraphicsBLT(x1, y1, height, width, (const char*)&image, color, xormode, offset, rs);

            return width;
        }
        case 2:
        {
            auto width = char_width_table[c];
            auto height = 7;
            auto image = font2_table[c].data();

            GraphicsBLT(x1, y1, height, width, (const char*)image, color, xormode, offset, rs);

            return width;
        }
        case 3:
        {
            auto width = char_width_table[c];
            auto height = 9;
            auto image = font3_table[c].data();

            GraphicsBLT(x1, y1, height, width, (const char*)image, color, xormode, offset, rs);

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
                pc.bgColor = src;
            }

            if ((*img) & (1<<(15-n)))
            {
                if(xormode) {
                    auto xored = src ^ (color&0xF);

                    if(srcPc.content == TextPixel)
                    {
                        srcPc.bgColor = srcPc.bgColor ^ (color & 0xf);
                        srcPc.fgColor = srcPc.fgColor ^ (color & 0xf);
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

