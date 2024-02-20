#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Magnum/Animation/Player.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Quaternion.h>
#include <Magnum/Math/Complex.h>

#include <stdint.h>
#include <assert.h>

#include <array>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <future>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <deque>
#include <chrono>

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

    bool operator!=(const vec2& other) const {
        return x != other.x || y != other.y;
    }

    vec2& operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    vec2 operator-(const vec2& other) const {
        return vec2(x - other.x, y - other.y);
    }

    vec2() : x(0), y(0) {}
    vec2(T _x, T _y) : x(_x), y(_y) {}
};

template<typename T>
struct vec3 {
    T x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    static vec3 lerp(const vec3& start, const vec3& end, float t) {
        return vec3(
            start.x + (end.x - start.x) * t,
            start.y + (end.y - start.y) * t,
            start.z + (end.z - start.z) * t
        );
    }

    static vec3 slerp(const vec3& start, const vec3& end, float t) {
        T dot = start.x * end.x + start.y * end.y + start.z * end.z;
        const T THRESHOLD = static_cast<T>(0.9995);
        if (dot > THRESHOLD) {
            vec3<T> result = start + (end - start) * t;
            result.x = result.x + (end.x - start.x) * t;
            result.y = result.y + (end.y - start.y) * t;
            result.z = result.z + (end.z - start.z) * t;
            T invLen = static_cast<T>(1.0) / sqrt(result.x * result.x + result.y * result.y + result.z * result.z);
            result.x *= invLen;
            result.y *= invLen;
            result.z *= invLen;
            return result;
        }

        dot = std::clamp(dot, static_cast<T>(-1.0), static_cast<T>(1.0));
        T theta_0 = acos(dot);
        T theta = theta_0 * t;
        T sin_theta = sin(theta);
        T sin_theta_0 = sin(theta_0);

        T s0 = cos(theta) - dot * sin_theta / sin_theta_0;
        T s1 = sin_theta / sin_theta_0;

        vec3<T> result = (start * s0) + (end * s1);
        return result;
    }

    static vec3 normalize(const vec3& v) {
        T len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        // Avoid division by zero
        if (len == 0) return vec3(0, 0, 0);
        return vec3(v.x / len, v.y / len, v.z / len);
    }

    vec3 normalize() const {
        T len = sqrt(x * x + y * y + z * z);
        // Avoid division by zero
        if (len == 0) return vec3(0, 0, 0);
        return vec3(x / len, y / len, z / len);
    }

    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3 operator*(T scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    vec3 operator*(const vec3& other) const {
        return vec3(x * other.x, y * other.y, z * other.z);
    }
};

class CountingSemaphore {
public:
    CountingSemaphore(int maxCount) : maxCount(maxCount), count(0) {}

    int acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            cv.wait(lock);
        }
        --count;

        printf("CountingSemaphore: count %d\n", count);

        return count;
    }

    void release() {
        std::unique_lock<std::mutex> lock(mtx);
        if(count < maxCount) {
            ++count;
        }
        cv.notify_one();
    }

    void releaseAll() {
        std::unique_lock<std::mutex> lock(mtx);
        count = maxCount;
        cv.notify_all();
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int maxCount;
    int count;
};

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
    AuxSysPixel,
    StarMapPixel,
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
    float screenX;
    float screenY;
    float adjust;
    float planetSize;

    float orbitCamX;
    float orbitCamY;
    float orbitCamZ;

    float nebulaBase;
    float nebulaMultiplier;

    uint32_t orbitMask;

    float zoomLevel; // 8 means 8x8 pixels are shown 1:1 with the emulator
};

struct ShaderIcon {
    uint32_t isActive;
    float x;
    float y;
    float screenX;

    float screenY;
    float bltX;
    float bltY;
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
    float x;
    float y;
    float screenX;
    float screenY;

    float bltX;
    float bltY;
    uint32_t id;
    uint32_t clr;

    uint32_t icon_type;
    uint32_t iaddr;
    int32_t planet_to_sunX;
    int32_t planet_to_sunY;

    uint32_t seed;
};

struct StarMapSetup {
    std::vector<Icon> starmap;
    vec2<int16_t> offset;
    vec2<int16_t> window;
};

extern std::vector<Icon> GetLocalIconList(uint32_t* gameContext);

template<size_t N>
struct IconUniform {
    static_assert(N > 0, "IconUniform must contain at least one icon.");
    ShaderIcon icons[N];

    static void ConvertIconToShaderIcon(const Icon& icon, ShaderIcon& shaderIcon)
    {
        shaderIcon.x = icon.x;
        shaderIcon.y = icon.y;
        shaderIcon.screenX = icon.screenX;
        shaderIcon.screenY = icon.screenY;
        shaderIcon.bltX = icon.bltX;
        shaderIcon.bltY = icon.bltY;
        shaderIcon.id = icon.id;
        shaderIcon.clr = icon.clr;
        shaderIcon.icon_type = icon.icon_type;
        shaderIcon.planet_to_sunX = icon.planet_to_sunX;
        shaderIcon.planet_to_sunY = icon.planet_to_sunY;
        shaderIcon.planetIndex = IndexFromSeed(icon.seed);
        shaderIcon.isActive = 1; // Assuming the icon is active when converted
    }

    IconUniform() : icons{}  {}

    IconUniform(std::vector<Icon> _icons)
    {
        assert(_icons.size() <= N);

        for(int i = 0; i < N; ++i)
        {
            if(i < _icons.size())
            {
                ConvertIconToShaderIcon(_icons.at(i), icons[i]);
            }
            else
            {
                icons[i].isActive = 0;
            }
        }
    }

    IconUniform(Icon planet)
    {
        for(int i = 0; i < N; ++i)
        {
            icons[i].isActive = 0;
        }

        ConvertIconToShaderIcon(planet, icons[0]);
    }

    static uint32_t IndexFromSeed(uint32_t seed);
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
    uint32_t navMask;
    uint32_t chromaKey;

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
        assert(pixel == ClearPixel || pixel == PlotPixel || pixel == PolyFillPixel || pixel == TilePixel || pixel == EllipsePixel || pixel == RunBitPixel || pixel == AuxSysPixel || pixel == StarMapPixel);
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
                case AuxSysPixel:
                case StarMapPixel:
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
        assert(pixel == ClearPixel || pixel == PlotPixel || pixel == PolyFillPixel || pixel == TilePixel || pixel == EllipsePixel || pixel == AuxSysPixel || pixel == StarMapPixel);
        content = pixel;
        EGAcolor = 0;
        argb = 0;
        blt_x = 0;
        blt_y = 0;
        return *this;
    }
};

struct FrameToRender
{
    std::vector<Icon> iconList;
    vec2<int16_t> worldCoord;
    vec2<int16_t> screenCoord;
    vec2<int16_t> deadReckoning;
    int16_t heading;
    uint32_t renderCount;
    uint16_t orbitMask;

    std::vector<Icon> solarSystem;
    StarMapSetup starMap;
};

enum class OrbitState {
    Holding,
    Insertion,
    Orbiting,
    Landing,
    Landed,
    Takeoff
};

struct OrbitStatus {
    vec3<float> camPos;
    float apparentSphereSize;
};

struct FrameSync {
    std::mutex mutex;
    std::condition_variable frameCompleted;
    uint64_t completedFrames = 0;

    // V= CONTEXT-ID#   ( 0=planet surface, 1=orbit, 2=system)         
    // (3 = hyperspace, 4 = encounter, 5 = starport)
    uint32_t gameContext = 0;

    uint32_t currentPlanet;
    uint32_t currentPlanetMass;
    uint32_t currentPlanetSphereSize;

    bool maneuvering = false;
    std::chrono::steady_clock::time_point maneuveringStartTime;
    std::chrono::steady_clock::time_point maneuveringEndTime;

    float shipHeading = 0.0;

    bool inGameOps = false;
    bool inNebula = false;
    bool inDrawAuxSys = false;
    bool inDrawShipButton = false;
    bool inSmallLogo = false;
    bool inDrawStarMap = false;
    bool inCombatRender = false;
    bool shouldSave = false;

    inline static const vec3<float> staringPos = { 0.0f, -0.918f, 0.397f };

    int32_t gameTickTimer = 0;

    std::deque<FrameToRender> framesToRender;

    FrameToRender stoppedFrame;

    std::chrono::steady_clock::time_point lastNonMovingArrowKey;

    OrbitState orbitState = OrbitState::Holding;
    std::chrono::steady_clock::time_point orbitTimestamp;
    std::optional<vec3<float>> orbitCamPos = std::nullopt;

    Magnum::Animation::Track<Magnum::Float, Magnum::Vector3> positionTrack{};

    Magnum::Animation::Track<Magnum::Float, Magnum::Float> xTrack{};
    Magnum::Animation::Track<Magnum::Float, Magnum::Float> yTrack{};
    Magnum::Animation::Track<Magnum::Float, Magnum::Float> zTrack{};

    Magnum::Animation::Track<Magnum::Float, Magnum::Float> scaleTrack{};

    void SetOrbitState(OrbitState state, std::optional<vec3<float>> optionalCamPos = std::nullopt);
    OrbitStatus GetOrbitStatus();

};

extern FrameSync frameSync;

// Declare the promise and future as extern so they can be defined elsewhere
extern std::promise<void> initPromise;
extern std::future<void> initFuture;

static const int CGAToEGA[16] = {0, 2, 1, 9, 4, 8, 5, 11, 6, 10, 7, 3, 6, 14, 12, 15};
static const int EGAToCGA[16] = {0, 2, 1, 11, 4, 6, 8, 10, 5, 3, 9, 7, 14, 0, 13, 15};
static const int NagivationWindowWidth = 72;
static const int NagivationWindowHeight = 120;

extern uint32_t colortable[16];
extern bool graphicsIsShutdown;

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

void GraphicsSetDeadReckoning(int16_t deadX, int16_t deadY, const std::vector<Icon>& iconList, const std::vector<Icon>& system, uint16_t orbitMask, const StarMapSetup& starMap);
void GraphicsReportGameFrame();
void GraphicsSetOrbitState(OrbitState state, std::optional<vec3<float>> optionalCamPos = std::nullopt);

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
