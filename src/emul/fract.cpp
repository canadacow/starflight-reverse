#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <iostream>

#include "../util/lodepng.h"

//#ifndef DEBUG
#define USE_INLINE_MEMORY
//#endif

#include "fract.h"
#include "starsystem.h"

extern "C" {
    __declspec(dllimport) void __stdcall OutputDebugStringA(const char* lpOutputString);
}

#if defined(__GNUC__)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE inline
#endif

const unsigned short int cc_FNULL = 0xe364;
const unsigned short int cc_MERCATOR_dash_SCALE = 0xe386;
const unsigned short int cc_CONTOUR_dash_SCALE = 0xe7fd;
const unsigned short int pp_X0_i_ = 0xe370;
const unsigned short int pp_Y0_i_ = 0xe374;
const unsigned short int pp_XYANCHOR = 0xe356; 
const unsigned short int pp_XCON = 0x5916;
const unsigned short int pp_YCON = 0x5921;
const unsigned short int pp_SPHEREWRAP = 0x4cca;
const unsigned short int pp_SIGNEXTEND = 0x4cd8;
const unsigned short int pp_X2 = 0x562e;
const unsigned short int pp_Y2 = 0x5625;
const unsigned short int pp_STD = 0xe35a;
const unsigned short int pp_RATIO = 0xe35e;
const unsigned short int pp_XLL = 0x4e49;
const unsigned short int pp_YLL = 0x4e53;
const unsigned short int pp_XUR = 0x4e5d;
const unsigned short int pp_YUR = 0x4e67;
const unsigned short int pp_GLOBALSEED = 0x5979;
const unsigned short int pp_SEED = 0x4ab0;
const unsigned short CONTOUR_RATIO_1 = 20882;
const unsigned short CONTOUR_RATIO_2 = 32767;
const unsigned short CONTOUR_SCALE = 20;

const unsigned short MERCATOR_RATIO_1 = 20882;
const unsigned short MERCATOR_RATIO_2 = 32767;
const unsigned short MERCATOR_SCALE = 117;

#pragma pack(push, 1)
typedef struct
{
    unsigned short int width;
    unsigned short int height;
    unsigned short rowTable;
    unsigned short segment;
} ArrayType;
#pragma pack(pop)

struct FractalState {
    // Original memory locations shown in comments
    
    // Core fractal parameters
    uint16_t x_lower_left;    // regbp+8
    uint16_t y_lower_left;    // regbp+6  
    uint16_t x_upper_right;   // regbp+4
    uint16_t y_upper_right;   // regbp+2
    uint16_t std;             // regbp+0xe (standard deviation/roughness)
    
    // Midpoint coordinates calculated in MIDPT()
    uint16_t x_mid;           // regbp+0xC
    uint16_t y_mid;           // regbp+0xA
    
    // Control flags
    uint16_t dx_greater_than_one;  // 0xe36c DX>1
    uint16_t dy_greater_than_one;  // 0xe368 DY>1
    
    // Ratio values used in NEWSTD()
    uint16_t ratio1;          // 0xe35e RATIO
    uint16_t ratio2;          // 0xe360 RATIO
    
    // Temporary storage
    uint16_t temp_y;          // 0xe396 TY

    int recursion_depth;
    
    // Array and wrapping control
    //uint16_t xy_anchor;       // 0xe356 XYANCHOR
    //uint16_t sphere_wrap;     // 0x4cca SPHEREWRAP
    //uint16_t sign_extend;     // 0x4cd8 SIGNEXTEND
    
    // Original coordinate storage
    //uint16_t x0;             // 0xe370 X0'
    //uint16_t y0;             // 0xe374 Y0'
};

ArrayType MERCATOR = {0x0030, 0x0018, 0x0480, 0x7e51};
ArrayType CONANCHOR = {0x0009, 0x0007, 0x003f, 0x7e4c};
ArrayType CONTOUR = {0x003d, 0x0065, 0x1811, 0x7cbe};

ArrayType* GlobalArrayDescriptor = nullptr;

static const uint16_t MERCATOR_ARRAY = 0x6a99;
static const uint16_t CONANCHOR_ARRAY = 0x6aad;
static const uint16_t CONTOUR_ARRAY = 0x6ac1;

void Store()
{
    unsigned short address = Pop(); // Get the address to store the value at
    unsigned short value = Pop();   // Get the value to store
    Write16(address, value);        // Write the value to the specified address
}

void _2_ex__2() // 2!_2
{
    // Takes 3 params: address, value1, value2
    // Stores value1 at address, value2 at address+2
    unsigned short address = Pop();
    unsigned short val1 = Pop(); 
    unsigned short val2 = Pop();
    
    Write16(address, val1);
    Write16(address + 2, val2); 
}

void SWAP()
{
    unsigned short a = Pop();
    unsigned short b = Pop();
    Push(a);
    Push(b);
}

void _slash_() // /
{
    // If stack is [a, b] (b on top)
    unsigned short b = Pop();  // divisor
    unsigned short a = Pop();  // dividend
    Push(a / b);              // result
}

void MOD() // MOD
{
    // If stack is [a, b] (b on top)
    unsigned short b = Pop();  // divisor
    unsigned short a = Pop();  // dividend
    Push(a % b);              // remainder
}

void ON_3() // ON_3
{
  Push(0xffff);
  SWAP(); // SWAP
  Store(); // !
}

void OFF_2() // OFF_2
{
    unsigned short bx = Pop();
    Write16(bx, 0);
}

void OVER()
{
    // If stack is [a, b] (where b is on top)
    // After OVER it becomes [a, b, a]
    unsigned short b = Pop();  // Get top value
    unsigned short a = Pop();  // Get second value
    Push(a);                  // Put second value back
    Push(b);                  // Put top value back
    Push(a);                  // Put copy of second value on top
}

void _2OVER()
{
    // If stack is [a, b, c, d] (d on top)
    unsigned short d = Pop();
    unsigned short c = Pop();
    unsigned short b = Pop();
    unsigned short a = Pop();
    // Restore original stack and add copies of a,b
    Push(a);
    Push(b);
    Push(c);
    Push(d);
    Push(a);
    Push(b);
    // Stack is now [a, b, c, d, a, b]
}

void _2DUP()
{
    // If stack is [... a, b] (b on top)
    unsigned short b = Pop();
    unsigned short a = Pop();
    Push(a);
    Push(b);
    Push(a);
    Push(b);
    // Stack becomes [... a, b, a, b]
}

void ROT()
{
    // If stack is [... a, b, c] (c on top)
    unsigned short c = Pop();
    unsigned short b = Pop();
    unsigned short a = Pop();
    Push(b);
    Push(c);
    Push(a);
    // Stack becomes [... b, c, a]
}

void IsUPDATE() // ?UPDATE
{
    // No-op for our purposes
}

void OFF() // OFF
{
    IsUPDATE(); // ?UPDATE
    OFF_2(); // OFF_2
}

void _2_ex_() // 2!
{
    IsUPDATE(); // ?UPDATE
    _2_ex__2(); // 2!_2
}

void LC_ex_() // LC!
{
    unsigned short bx = Pop();    // Get address offset
    unsigned short ds = Pop();    // Get segment
    unsigned char al = Pop();     // Get value to store
    
    // Write byte value to segment:offset
    Write8Long(ds, bx, al);
}

void LC_at_() // LC@
{
    unsigned short bx = Pop();    // Get address offset
    unsigned short ds = Pop();    // Get segment
    
    // Read byte value from segment:offset and zero-extend to word
    unsigned char value = Read8Long(ds, bx);
    Push((unsigned short)value);
}

//#define USE_OG_READARRAY 1

#if defined(USE_OG_READARRAY)
void ReadArray(ArrayType& array) {
    // Get indices from stack
    unsigned short row = Pop();    // i * 2
    unsigned short col = Pop();    // j * 4
    
    // Calculate offset into array
    unsigned short offset = array.bx + (row * array.width + col);
    
    // Push segment:offset address for LC_ex_
    Push(array.ds);    // segment
    Push(offset);      // offset
}
#else
void ReadArray(uint16_t arrayDescriptor) {
    // Get indices from stack
    unsigned short y = Pop();    // row index
    unsigned short x = Pop();    // column index
    
    // Get array descriptor (similar to ACELLADDR)
    unsigned short segmentAddr = Read16(arrayDescriptor + 6);
    unsigned short rowTable = Read16(arrayDescriptor + 4);

    // Calculate row offset using lookup table
    unsigned short rowIndex = y << 1;            // Multiply y by 2 for word indexing
    unsigned short rowOffset = Read16Long(segmentAddr, rowTable + rowIndex);

    // Calculate final offset
    unsigned short finalOffset = rowOffset + x;

    // Push segment:offset address for LC_ex_
    Push(segmentAddr);    // segment
    Push(finalOffset);    // offset
}
#endif

void SETLARRAY() // SETLARRAY
{
  Push(0x4cf1); // 'ARRAY
  Store(); // !

  static uint32_t arrayDescOffset = ComputeAddress(StarflightBaseSegment, 0x4cf1);

  const uint16_t arrayDesc = *reinterpret_cast<const uint16_t*>(&currentMemory[arrayDescOffset]);
  uint32_t arrayDescAddress = ComputeAddress(StarflightBaseSegment, arrayDesc);
  GlobalArrayDescriptor = reinterpret_cast<ArrayType*>(&currentMemory[arrayDescAddress]);

  Push(0x4d5c); // 'W4D5C'
  Push(0x4cdc); // W4CDC
  Store(); // !
  Push(0x2ee5); // 'LC!'
  Push(0x4ce0); // W4CE0
  Store(); // !
  Push(Read16(pp_SIGNEXTEND)); // SIGNEXTEND @
  if (Pop() != 0)
  {
    Push(0x49f0); // 'L+-@'
  } else
  {
    Push(0x2ecd); // 'LC@'
  }
  Push(0x4ce4); // W4CE4
  Store(); // !
}

void Store_3() // !_3
{
  IsUPDATE(); // ?UPDATE
  Store(); // !
}

FORCE_INLINE void RRND()
{
    unsigned short ax, bx, cx, dx;
    ax = *RandomSeed; // SEED
    cx = 0x7abd;
    ax = ((signed short)cx) * ((signed short)ax);
    ax += 0x1b0f;
    *RandomSeed = ax; // SEED

    bx = Pop(); // range
    cx = Pop(); // low
    bx -= cx;
    unsigned int mul = ((unsigned int)ax) * ((unsigned int)bx);
    dx = ((mul >> 16)&0xFFFF) + cx;
    Push(dx);
}

FORCE_INLINE uint16_t RRND_WITHRES(uint16_t low, uint16_t range)
{
    unsigned short ax, bx, cx, dx;
    ax = *RandomSeed; // SEED
    cx = 0x7abd;
    ax = ((signed short)cx) * ((signed short)ax);
    ax += 0x1b0f;
    *RandomSeed = ax; // SEED

    bx = range; // Pop()
    cx = low; // Pop()
    bx -= cx;
    unsigned int mul = ((unsigned int)ax) * ((unsigned int)bx);
    dx = ((mul >> 16)&0xFFFF) + cx;
    return dx; // Push(dx)
}

FORCE_INLINE int16_t C_PLUS_LIMIT(int16_t a, int16_t b) {
    int16_t sum = a + b;
    return std::clamp(sum, int16_t(-0x7F), int16_t(0x7F));
}

FORCE_INLINE uint16_t DISPLACEMENT(uint16_t x, uint16_t y, FractalState& fractalState) {
    
    if (Read16(0xe356)) { // XYANCHOR
        // Adjust coordinates
        x += Read16(pp_X0_i_);  // X0'
        y += Read16(pp_Y0_i_);  // Y0'
        
        // Handle Y wrapping
        if (y & 0x8000) {
            y = -(y + 1);
            x += 0x0480;
        } else {
            uint16_t heightLimit = 0x3c0;
            if (heightLimit <= y) {
                y = heightLimit - y + heightLimit + 1;
                x += 0x480;
            }
        }
        
        // First random sequence
        *RandomSeed = y;  // First seed update
        uint16_t savedX = x;  // Save x for later seed restore
        
        uint16_t rand1 = RRND_WITHRES(1, -1);
        
        // Restore and save seed
        *RandomSeed = savedX;  // Second seed update
        
        // Generate second random number
        uint16_t rand2 = RRND_WITHRES(1, -1);
        
        // Final seed calculation
        *RandomSeed = rand2 ^ Read16(pp_GLOBALSEED) ^ rand1;  // Third seed update
    }
    
    // Generate final displacement value
    uint16_t range = fractalState.std;
    uint16_t displacement = RRND_WITHRES(-range, range);  // Generate random value in range [-std, std]
    return displacement;
}

FORCE_INLINE void SWRAP_REF(uint16_t& x, uint16_t& y, const ArrayType* arrayDescriptor)
{
    signed short ax = y;
    signed short cx = x;
    signed short bx;
    if (ax < 0)
    {
        ax = -(ax + 1);
        bx = arrayDescriptor->width >> 1; // 'ARRAY
        cx += bx;
    }
    else
    {
        bx = arrayDescriptor->height; // 'ARRAY
        if ((unsigned short)bx <= (unsigned short)ax)
        {
            ax = bx - ax + bx + 1;
            bx = arrayDescriptor->width; // 'ARRAY
            bx = bx >> 1;
            cx += bx;
        }
    }

    if (cx < 0)
    {
        cx += arrayDescriptor->width; // 'ARRAY
    }
    else
    {
        bx = arrayDescriptor->width; // 'ARRAY
        if ((unsigned short)bx <= (unsigned short)cx)
        {
            cx -= bx;
        }
    }
    x = cx;
    y = ax;
}

FORCE_INLINE void SWRAP()
{
    signed short ax, cx, bx;
    ax = Pop(); // y
    cx = Pop(); // x
    if (ax < 0)
    {
        ax = -(ax+1);
        bx = Read16(Read16(0x4CF1)) >> 1; // 'ARRAY
        cx += bx;
    } else
    {
        bx = Read16(Read16(0x4CF1)+2); // 'ARRAY
        if ((unsigned short)bx <= (unsigned short)ax)
        {
            ax = bx - ax + bx + 1;
            bx = Read16(Read16(0x4CF1)); // 'ARRAY
            bx = bx >> 1;
            cx += bx;
        }
    }

    if (cx < 0)
    {
        cx += Read16(Read16(0x4CF1)); // 'ARRAY
    } else
    {
        bx = Read16(Read16(0x4CF1)); // 'ARRAY
        if ((unsigned short)bx <= (unsigned short)cx) {
            cx -= bx;
        }
    }
    Push(cx);
    Push(ax);
}

FORCE_INLINE uint16_t GetArrayDescriptor() {
    return Read16(0x4cf1); // 'ARRAY
}

FORCE_INLINE bool IsSphereWrap() {
    return Read16(pp_SPHEREWRAP) != 0; // SPHEREWRAP
}

FORCE_INLINE bool IsSignExtend() {
    return Read16(pp_SIGNEXTEND) != 0; // SIGNEXTEND  
}

FORCE_INLINE void ACELLADDR_TO_SEG_OFF(uint16_t& segment, uint16_t& offset) {
    segment = GlobalArrayDescriptor->segment;
    const uint16_t rowTable = GlobalArrayDescriptor->rowTable;
    
    // Get coordinates from stack
    const uint16_t y = Pop();
    const uint16_t x = Pop();

    // Handle sphere wrapping if needed
    uint16_t finalX = x;
    uint16_t finalY = y;
    if (IsSphereWrap()) {
        SWRAP_REF(finalX, finalY, GlobalArrayDescriptor);
    }

    // Calculate final address
    const uint16_t rowIndex = finalY << 1;
    const uint16_t rowOffset = Read16Long(segment, rowTable + rowIndex);
    offset = rowOffset + finalX;
}

FORCE_INLINE void ACELLADDR_AND_GET() {
    uint16_t segment;
    uint16_t offset;
    ACELLADDR_TO_SEG_OFF(segment, offset);

    // Read the value and handle sign extension
    const uint8_t value = Read8Long(segment, offset);

    if (IsSignExtend()) {
        Push((int16_t)((int8_t)value));
    } else {
        Push((uint16_t)value);
    }
}

FORCE_INLINE void ACELLADDR_XY_TO_SEG_OFF(uint16_t x, uint16_t y, uint16_t& segment, uint16_t& offset) {
    segment = GlobalArrayDescriptor->segment;
    const uint16_t rowTable = GlobalArrayDescriptor->rowTable;
    
    // Handle sphere wrapping if needed
    uint16_t finalX = x;
    uint16_t finalY = y;
    if (IsSphereWrap()) {
        SWRAP_REF(finalX, finalY, GlobalArrayDescriptor);
    }

    // Calculate final address
    const uint16_t rowIndex = finalY << 1;
    const uint16_t rowOffset = Read16Long(segment, rowTable + rowIndex);
    offset = rowOffset + finalX;
}

FORCE_INLINE uint16_t ACELLADDR_XY_AND_GET(uint16_t x, uint16_t y) {
    uint16_t segment;
    uint16_t offset;
    ACELLADDR_XY_TO_SEG_OFF(x, y, segment, offset);

    // Read the value and handle sign extension
    const uint8_t value = Read8Long(segment, offset);

    uint16_t result;

    if (IsSignExtend()) {
        result = (uint16_t)((int16_t)((int8_t)value));
    } else {
        result = ((uint16_t)value);
    }

    return result;
}

FORCE_INLINE void ACELLADDR_AND_STORE() {
    uint16_t segment;
    uint16_t offset;
    ACELLADDR_TO_SEG_OFF(segment, offset);

    // Write the value to the calculated address
    const uint8_t value = (uint8_t)Pop();
    Write8Long(segment, offset, value);
}

FORCE_INLINE void ACELLADDR()
{
    unsigned short ax, bx, cx, dx;
       
    ax = Read16(0x4cca); // SPHEREWRAP
    if (ax) SWRAP();
    bx = Read16(0x4cf1); // 'ARRAY
    ax = Read16(bx+6);
    bx = Read16(bx+4);
    cx = Pop() << 1;
    bx += cx;
    cx = Read16Long(ax, bx);
    dx = Pop();
    cx += dx;
    Write16(0xe378, ax); // SCELL
    Write16(0xe37c, cx); // OCELL
}

FORCE_INLINE void AGet() // [A@]
{
    unsigned short ax, cx, temp;
    unsigned char al;
    al = Read8Long(Read16(0xe378), Read16(0xe37c)); // SCELL OCELL
    cx = Read16(0x4cD8); // SIGNEXTEND

    if (cx)
    {
        ax = (signed short)((signed char)al);
    } else
    {
        ax = al;
    }
    Push(ax);
}

FORCE_INLINE void A_ex_() // [A!]
{
    unsigned short value = Pop();
    // Write to same memory locations that AGet reads from
    Write8Long(Read16(0xe378), Read16(0xe37c), (unsigned char)value); // SCELL OCELL
}


void Readable_AV_dash_MIDPT(int x1, int y1, int x2, int y2) {
    // Get value at first point (x1,y1)
    Push(x1);
    Push(y1);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    int16_t val1 = Pop();

    // Get value at second point (x2,y2)
    Push(x2);
    Push(y2);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    int16_t val2 = Pop();

    // Calculate average value
    int16_t avgVal = (val1 + val2) >> 1;  // Average of the two points' values

    // Calculate midpoint coordinates
    int midX = (x1 + x2) >> 1;  // Midpoint X coordinate
    int midY = (y1 + y2) >> 1;  // Midpoint Y coordinate

    // Store average value at midpoint
    Push(avgVal & 0xFF);
    Push(midX);
    Push(midY);
    //ACELLADDR();
    //A_ex_();  // Store to memory (A!)
    ACELLADDR_AND_STORE();
}

void AV_dash_MIDPT()
{
    Readable_AV_dash_MIDPT(Pop(), Pop(), Pop(), Pop());
}

FORCE_INLINE void FRACT_StoreHeight(uint16_t x, uint16_t y, int16_t val) // Set Anchor
{
    if (GlobalArrayDescriptor == nullptr) {
        static constexpr uint32_t arrayDescOffset = ComputeAddress(StarflightBaseSegment, 0x4cf1);

        const uint16_t arrayDesc = *reinterpret_cast<const uint16_t*>(&currentMemory[arrayDescOffset]);
        uint32_t arrayDescAddress = ComputeAddress(StarflightBaseSegment, arrayDesc);
        GlobalArrayDescriptor = reinterpret_cast<ArrayType*>(&currentMemory[arrayDescAddress]);
    }

    unsigned short ax;
    uint16_t segment, offset;
    ACELLADDR_XY_TO_SEG_OFF(x, y, segment, offset); // Get segment and offset
    ax = Read8Long(segment, offset); // Read value using segment and offset
    if (ax == 0x80) // not set yet
    {
        Write8Long(segment, offset, val & 0xFF); // Write value using segment and offset
    }
}

void Ext_FRACT_StoreHeight() {
    GlobalArrayDescriptor = nullptr;

    uint16_t y = Pop();
    uint16_t x = Pop();
    int16_t val = Pop();
    FRACT_StoreHeight(x, y, val);
}

FORCE_INLINE void XSHIFT(uint16_t xVal, FractalState& fractalState)
{
    unsigned short ax, bx, cx;
    fractalState.temp_y = xVal; // TY

    cx = ACELLADDR_XY_AND_GET(fractalState.x_lower_left, fractalState.temp_y);
    ax = ACELLADDR_XY_AND_GET(fractalState.x_upper_right, fractalState.temp_y);

    ax += cx;
    ax = ((signed short)ax) >> 1;

    bx = fractalState.x_lower_left;
    bx += fractalState.x_upper_right;
    bx = ((signed short)bx) >> 1;

    uint16_t displacement = DISPLACEMENT(bx, fractalState.temp_y, fractalState);
    int16_t val = C_PLUS_LIMIT(displacement, ax);

    FRACT_StoreHeight(fractalState.x_mid, fractalState.temp_y, val);
}

FORCE_INLINE void YSHIFT(uint16_t yVal, FractalState& fractalState)
{
    unsigned short ax, bx, cx;

    fractalState.temp_y = yVal; // TY

    cx = ACELLADDR_XY_AND_GET(fractalState.temp_y, fractalState.y_lower_left);
    ax = ACELLADDR_XY_AND_GET(fractalState.temp_y, fractalState.y_upper_right);

    ax += cx;
    ax = ((signed short)ax) >> 1;

    bx = fractalState.y_lower_left;
    bx += fractalState.y_upper_right;
    bx = ((signed short)bx) >> 1;

    uint16_t displacement = DISPLACEMENT(fractalState.temp_y, bx, fractalState);
    int16_t val = C_PLUS_LIMIT(displacement, ax);

    FRACT_StoreHeight(fractalState.temp_y, fractalState.y_mid, val);
}

FORCE_INLINE void EDGES(FractalState& fractalState)
{
    unsigned short ax;
    ax = fractalState.dy_greater_than_one; // DY>1
    if (ax != 0)
    {
        ax = fractalState.x_lower_left;
        if (ax == 0) {
            YSHIFT(ax, fractalState);
        }
        YSHIFT(fractalState.x_upper_right, fractalState);
    }

    ax = fractalState.dx_greater_than_one; // DX>1
    if (ax != 0)
    {
        ax = fractalState.y_lower_left;
        if (ax == 0)
        {
            XSHIFT(ax, fractalState);
        }
        XSHIFT(fractalState.y_upper_right, fractalState);
    }
}

FORCE_INLINE void CENTER(FractalState& fractalState)
{
    unsigned short ax, cx;
    ax = fractalState.dy_greater_than_one & fractalState.dx_greater_than_one; // DY>1 and DX>1
    if (ax == 0) return;

    uint16_t val1 = ACELLADDR_XY_AND_GET(fractalState.x_mid, fractalState.y_lower_left);

    uint16_t val2 = ACELLADDR_XY_AND_GET(fractalState.x_mid, fractalState.y_upper_right);

    uint16_t val3 = ACELLADDR_XY_AND_GET(fractalState.x_lower_left, fractalState.y_mid);

    uint16_t val4 = ACELLADDR_XY_AND_GET(fractalState.x_upper_right, fractalState.y_mid);

    // calculate average
    ax = val1 + val2 + val3 + val4;
    ax = ((signed short)ax) >> 2;

    uint16_t displacement = DISPLACEMENT(fractalState.x_mid, fractalState.y_mid, fractalState);

    int16_t val = C_PLUS_LIMIT(displacement, ax);

    FRACT_StoreHeight(fractalState.x_mid, fractalState.y_mid, val);
}

void MIDPT(FractalState& fractalState)
{
    unsigned short ax;
    ax = fractalState.x_lower_left;
    ax += fractalState.x_upper_right;
    ax = ((signed short)ax) >> 1;
    fractalState.x_mid = ax;

    ax = fractalState.y_lower_left;
    ax += fractalState.y_upper_right;
    ax = ((signed short)ax) >> 1;
    fractalState.y_mid = ax;
}

FORCE_INLINE void NEWSTD(FractalState& fractalState)
{
    unsigned int ax = fractalState.std;
    unsigned int ratio1 = fractalState.ratio1; // RATIO
    unsigned int ratio2 = fractalState.ratio2; // RATIO
    ax = (ax * ratio2) / ratio1;
    if (((signed int)ax) <= 0) // not sure
    {
        ax = 1;
    }
    fractalState.std = ax;
}

void FRACTAL(FractalState fractalState)
{
    // Calculate DX>1 and DY>1
    fractalState.dx_greater_than_one = (fractalState.x_upper_right - fractalState.x_lower_left - 1) > 0 ? 1 : 0;

    fractalState.dy_greater_than_one = (fractalState.y_upper_right - fractalState.y_lower_left - 1) > 0 ? 1 : 0;

    if (fractalState.dx_greater_than_one > 0 || fractalState.dy_greater_than_one > 0)
    {
        // Calculate midpoints
        fractalState.x_mid = (fractalState.x_lower_left + fractalState.x_upper_right) / 2;
        fractalState.y_mid = (fractalState.y_lower_left + fractalState.y_upper_right) / 2;

        // Process edges and center
        EDGES(fractalState);
        CENTER(fractalState);
        NEWSTD(fractalState);

        // Prepare for recursive calls
        FractalState newFractalState = fractalState;
        newFractalState.recursion_depth++;

        // Recursive calls with different parameters
        // Quadrant 0: bottom-left
        newFractalState.x_upper_right = fractalState.x_mid;
        newFractalState.y_upper_right = fractalState.y_mid;
        FRACTAL(newFractalState);

        // Quadrant 1: bottom-right
        newFractalState.x_lower_left = fractalState.x_mid;
        newFractalState.x_upper_right = fractalState.x_upper_right;
        FRACTAL(newFractalState);

        // Quadrant 2: top-left
        newFractalState.x_lower_left = fractalState.x_lower_left;
        newFractalState.y_lower_left = fractalState.y_mid;
        newFractalState.x_upper_right = fractalState.x_mid;
        newFractalState.y_upper_right = fractalState.y_upper_right;
        FRACTAL(newFractalState);

        // Quadrant 3: top-right
        newFractalState.x_lower_left = fractalState.x_mid;
        newFractalState.y_lower_left = fractalState.y_mid;
        newFractalState.x_upper_right = fractalState.x_upper_right;
        newFractalState.y_upper_right = fractalState.y_upper_right;
        FRACTAL(newFractalState);
    }
}

void FRACT_FRACTALIZE()
{
    FractalState fractalState = {};

    unsigned short int yur, xur, yll, xll, dummy1, dummy2, std;
    fractalState.y_upper_right = Pop();
    fractalState.x_upper_right = Pop();
    fractalState.y_lower_left = Pop();
    fractalState.x_lower_left = Pop();
    Pop();
    Pop();
    fractalState.std = Pop();

    //printf("FRACTALIZE xll=%i yll=%i xur=%i yur=%i std=%i\n", xll, yll, xur, yur, std);
    //char debug_str[256];
    //sprintf(debug_str, "FRACTALIZE xll=%i yll=%i xur=%i yur=%i std=%i\n", xll, yll, xur, yur, std);
    //OutputDebugStringA(debug_str);

    fractalState.ratio1 = Read16(0xe35e); // RATIO
    fractalState.ratio2 = Read16(0xe360); // RATIO

    FRACTAL(fractalState);
}

void FRACT_FILLARRAY()
{
    unsigned short ax, bx, cx;
    ax = Pop();
    bx = Read16(0x4CF1); // 'ARRAY
    cx = Read16(bx+4);
    bx = Read16(bx+6);
    printf("FRACT_FILLARRAY count=%i es=0x%04x al=%i\n", cx, bx, ax&0xFF);
    for(int i=0; i<cx; i++) Write8Long(bx, i, ax&0xFF);
}

void FRACT_INIT_CONTOUR() // INIT-CONTOUR
{
  Push(0x6ac1); // 'CONTOUR'
  SETLARRAY(); // SETLARRAY
  Push(Read16(cc_FNULL)); // FNULL
  FRACT_FILLARRAY(); // FILLARRAY
}

void MERC_gt_CONANCHOR() {
    // Calculate grid dimensions
    int yOffset = Read16(pp_YCON) / 0x28;  // Divide YCON by 40 for Y offset
    int xOffset = Read16(pp_XCON) / 0x30;  // Divide XCON by 48 for X offset
    
    // Store grid offsets
    Push(yOffset);
    Push(pp_Y2);
    Store_3();  // Store Y2 offset
    
    Push(xOffset); 
    Push(pp_X2);
    Store_3();  // Store X2 offset

    // Set up array parameters
    Push(pp_SPHEREWRAP);
    ON_3();                    // Enable sphere wrapping
    Push(0x6a99);             // 'MERCATOR' constant
    SETLARRAY();              // Set up large array

    // Copy 4x3 grid of values from MERCATOR to CONANCHOR
    for(int i = 0; i < 4; i++) {           // Vertical loop
        for(int j = 0; j < 3; j++) {       // Horizontal loop
            // Get value from MERCATOR array
            int x_offset = Read16(pp_X2);
            int y_offset = Read16(pp_Y2);
            
            int x_coord = j + x_offset;
            int y_coord = y_offset + i;
            
            Push(x_coord);       // X coordinate 
            Push(y_coord);       // Y coordinate
            //ACELLADDR();
            //AGet();
            ACELLADDR_AND_GET();
            int value = Pop();

            Push(value);

            // Store in CONANCHOR array
            int x_index = j * 4;            // X index in CONANCHOR
            int y_index = i * 2;            // Y index in CONANCHOR
            Push(x_index);
            Push(y_index);
            ReadArray(CONANCHOR_ARRAY);           // Get CONANCHOR array address
            LC_ex_();                       // Store value in CONANCHOR
        }
    }
}

void CONANCHOR_dash_HOR() // CONANCHOR-HOR
{
  unsigned short int i, imax, j, jmax;

  i = 0;
  imax = 4;
  do // (DO)
  {

    j = 0;
    jmax = 2;
    do // (DO)
    {
      Push(j * 4); // I 4 *
      Push(i * 2); // J 2*
      OVER(); // OVER
      Push(Pop() + 4); //  4 +
      OVER(); // OVER
      AV_dash_MIDPT(); // AV-MIDPT
      Push(j * 4); // I 4 *
      Push(i * 2); // J 2*
      OVER(); // OVER
      Push(Pop() + 2); //  2 +
      OVER(); // OVER
      AV_dash_MIDPT(); // AV-MIDPT
      Push((j + 1) * 4); // I 1+ 4 *
      Push(i * 2); // J 2*
      OVER(); // OVER
      Push(Pop() - 2); //  2 -
      OVER(); // OVER
      AV_dash_MIDPT(); // AV-MIDPT
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}

void CONANCHOR_dash_VER() // CONANCHOR-VER
{
  unsigned short int i, imax, j, jmax;

  i = 0;
  imax = 3;
  do // (DO)
  {

    j = 0;
    jmax = 3;
    do // (DO)
    {
      Push(j * 4); // I 4 *
      Push(i * 2); // J 2*
      OVER(); // OVER
      OVER(); // OVER
      Push(Pop() + 2); //  2+
      AV_dash_MIDPT(); // AV-MIDPT
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}

void CONANCHOR_dash_CNT1() // CONANCHOR-CNT1
{
  unsigned short int i, imax, j, jmax;

  i = 0;
  imax = 3;
  do // (DO)
  {

    j = 0;
    jmax = 2;
    do // (DO)
    {
      Push(j * 4); // I 4 *
      Push(i * 2 + 1); // J 2* 1+
      OVER(); // OVER
      Push(Pop() + 4); //  4 +
      OVER(); // OVER
      AV_dash_MIDPT(); // AV-MIDPT
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}

void CONANCHOR_dash_CNT2() // CONANCHOR-CNT2
{
  unsigned short int i, imax, j, jmax;

  i = 0;
  imax = 3;
  do // (DO)
  {

    j = 0;
    jmax = 4;
    do // (DO)
    {
      Push(j * 2 + 1); // I 2* 1+
      Push(i * 2); // J 2*
      OVER(); // OVER
      OVER(); // OVER
      Push(Pop() + 2); //  2+
      AV_dash_MIDPT(); // AV-MIDPT
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}

void SETRELORIGIN() // SETRELORIGIN
{
  Push(pp_Y0_i_); // Y0'
  Store_3(); // !_3
  Push(pp_X0_i_); // X0'
  Store_3(); // !_3
}

void CONANCHOR_gt_CONTOUR() // CONANCHOR>CONTOUR
{
  unsigned short int a, i, imax, j, jmax;
  Push(Read16(pp_XCON)); // XCON @
  Push(0x0030);
  MOD(); // MOD
  Push(0x000c);
  _slash_(); // /
  Push(Read16(pp_YCON)); // YCON @
  Push(0x0028);
  MOD(); // MOD
  Push(0x0014);
  _slash_(); // /
  OVER(); // OVER
  Push(Read16(pp_XCON)); // XCON @
  Push(0x0030);
  _slash_(); // /
  Push(Pop() * 0x0030); //  0x0030 *
  SWAP(); // SWAP
  Push(Pop() * 0x000c); //  0x000c *
  Push(Pop() + Pop()); // +
  a = Pop(); // >R
  Push(Read16(pp_YCON)); // YCON @
  Push(0x0028);
  _slash_(); // /
  Push(Pop() * 0x0028); //  0x0028 *
  OVER(); // OVER
  Push(Pop() * 0x0014); //  0x0014 *
  Push(Pop() + Pop()); // +
  Push(a); // R>
  SWAP(); // SWAP
  SETRELORIGIN(); // SETRELORIGIN

  i = 0;
  imax = 6;
  do // (DO)
  {

    j = 0;
    jmax = 6;
    do // (DO)
    {
      OVER(); // OVER
      Push(Pop() + j); //  I +
      OVER(); // OVER
      Push(Pop() + i); //  J +
      ReadArray(CONANCHOR_ARRAY); // CONANCHOR
      LC_at_(); // LC@
      Push(j * 0x000c); // I 0x000c *
      Push(i * 0x0014); // J 0x0014 *
      ReadArray(CONTOUR_ARRAY); // CONTOUR
      LC_ex_(); // LC!
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

  Pop(); Pop(); // 2DROP
}


void FRACT_ANCHOR_CONTOUR() // ANCHOR_CONTOUR
{
  MERC_gt_CONANCHOR(); // MERC>CONANCHOR
  Push(0x6aad); // 'CONANCHOR'
  SETLARRAY(); // SETLARRAY
  CONANCHOR_dash_HOR(); // CONANCHOR-HOR
  CONANCHOR_dash_VER(); // CONANCHOR-VER
  CONANCHOR_dash_CNT1(); // CONANCHOR-CNT1
  CONANCHOR_dash_CNT2(); // CONANCHOR-CNT2
  CONANCHOR_gt_CONTOUR(); // CONANCHOR>CONTOUR
}

void SETSCALE() // SETSCALE
{
  Push(pp_STD); // STD
  Store_3(); // !_3
  Push(pp_RATIO); // RATIO
  _2_ex_(); // 2!
}

void SETREGION() // SETREGION
{
  Push(pp_YUR); // YUR
  Store(); // !
  Push(pp_XUR); // XUR
  Store(); // !
  Push(pp_YLL); // YLL
  Store(); // !
  Push(pp_XLL); // XLL
  Store(); // !
}

void FRACT_dash_REGION() // FRACT-REGION
{
  Push(Read16(pp_STD)); // STD @
  Push(0);
  Push(0);
  Push(Read16(pp_XLL)); // XLL @
  Push(Read16(pp_YLL)); // YLL @
  Push(Read16(pp_XUR)); // XUR @
  Push(Read16(pp_YUR)); // YUR @
  FRACT_FRACTALIZE(); // FRACTALIZE
}

#define USE_OG_SUB_CON_FRACT 0

#if defined(USE_OG_SUB_CON_FRACT)
void SUB_dash_CON_dash_FRACT() // SUB-CON-FRACT
{
  unsigned short int a;
  Push(Pop() * 0x0014); //  0x0014 *
  SWAP(); // SWAP
  Push(Pop() * 0x000c); //  0x000c *
  SWAP(); // SWAP
  OVER(); // OVER
  Push(Pop() + 1); //  1+
  OVER(); // OVER
  Push(Pop() + 1); //  1+
  //ACELLADDR(); // ACELLADDR
  //AGet(); // A@
  ACELLADDR_AND_GET();
  Push(Pop()==Read16(cc_FNULL)?1:0); //  FNULL =
  a = Pop(); // >R
  OVER(); // OVER
  Push(Pop() + 0x000b); //  0x000b +
  OVER(); // OVER
  Push(Pop() + 0x0013); //  0x0013 +
  //ACELLADDR(); // ACELLADDR
  //AGet(); // A@
  ACELLADDR_AND_GET();
  Push((Pop()==Read16(cc_FNULL)?1:0) & a); //  FNULL = R> AND
  if (Pop() != 0)
  {
    OVER(); // OVER
    Push(Pop() + 0x000c); //  0x000c +
    OVER(); // OVER
    Push(Pop() + 0x0014); //  0x0014 +
    SETREGION(); // SETREGION
    FRACT_dash_REGION(); // FRACT-REGION
    return;
  }
  Pop(); Pop(); // 2DROP
}

#else

void SUB_dash_CON_dash_FRACT(int x, int y) {
    // Scale coordinates to grid units
    int scaledY = y * 0x14;  // y * 20
    int scaledX = x * 0x0C;  // x * 12

    // Check first position (x+1, y+1)
    Push(scaledX + 1);
    Push(scaledY + 1);
    ACELLADDR();
    AGet();
    bool firstIsFNull = (Pop() == Read16(cc_FNULL));

    // Check second position (x+11, y+19)
    Push(scaledX + 0x0B);  // x + 11
    Push(scaledY + 0x13);  // y + 19
    ACELLADDR();
    AGet();
    
    // Only proceed if both positions contain FNULL
    if (firstIsFNull && (Pop() == Read16(cc_FNULL))) {
        // Set up region for fractal generation
        Push(scaledX + 0x0C);  // x + 12
        Push(scaledY + 0x14);  // y + 20
        SETREGION();
        FRACT_dash_REGION();
        return;
    }
}

#endif

void FRACT_FRACT_CONTOUR() // FRACT_CONTOUR
{
  unsigned short int i, imax, j, jmax;
  Push(pp_SPHEREWRAP); // SPHEREWRAP
  OFF(); // OFF
  Push(pp_SIGNEXTEND); // SIGNEXTEND
  ON_3(); // ON_3
  Push(0x6ac1); // 'CONTOUR'
  SETLARRAY(); // SETLARRAY
  Push(pp_XYANCHOR); // XYANCHOR
  ON_3(); // ON_3
  //Push2Words("CONTOUR-RATIO");
  Push(CONTOUR_RATIO_1);
  Push(CONTOUR_RATIO_2);
  Push(CONTOUR_SCALE); // CONTOUR-SCALE
  SETSCALE(); // SETSCALE

  i = 0;
  imax = 5;
  do // (DO)
  {

    j = 0;
    jmax = 5;
    do // (DO)
    {
      #if defined(USE_OG_SUB_CON_FRACT)
      Push(j); // I
      Push(i); // J
      SUB_dash_CON_dash_FRACT(); // SUB-CON-FRACT
      #else
      SUB_dash_CON_dash_FRACT(j, i); // SUB-CON-FRACT
      #endif
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

}

void FRACT_NEWCONTOUR() // NEWCONTOUR
{
  FRACT_INIT_CONTOUR(); // INIT-CONTOUR
  FRACT_ANCHOR_CONTOUR(); // ANCHOR_CONTOUR
  FRACT_FRACT_CONTOUR(); // FRACT_CONTOUR
}

/*
{
    std::ofstream cache("sf1_planet_surfaces.bin", std::ios::binary);
    size_t mapSize = surfaces.size();
    cache.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    
    for (const auto& pair : surfaces) {
        cache.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
        
        size_t size = pair.second.native.size();
        cache.write(reinterpret_cast<const char*>(&size), sizeof(size));
        cache.write(reinterpret_cast<const char*>(pair.second.native.data()), size);
    }
}
*/

bool FractalGenerator::Initialize(const std::filesystem::path& planetDatabase)
{
    if (initialized)
    {
        return true;
    }

    std::ifstream file(planetDatabase, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Could not open file " << planetDatabase << std::endl;
        return false;
    }

    // Read the map size
    size_t mapSize;
    file.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    if (file.fail())
    {
        std::cerr << "Failed to read map size from file " << planetDatabase << std::endl;
        return false;
    }

    // Read each surface entry
    for (size_t i = 0; i < mapSize; ++i)
    {
        uint32_t key;
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        if (file.fail())
        {
            std::cerr << "Failed to read key from file " << planetDatabase << std::endl;
            return false;
        }

        size_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (file.fail())
        {
            std::cerr << "Failed to read size from file " << planetDatabase << std::endl;
            return false;
        }

        std::vector<int8_t> native(size);
        file.read(reinterpret_cast<char*>(native.data()), size);
        if (file.fail())
        {
            std::cerr << "Failed to read native data from file " << planetDatabase << std::endl;
            return false;
        }

        nativeImages.emplace(key, std::move(native));
    }

    file.close();
    initialized = true;
    return true;
}

static uint32_t colortable[16] =
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


static uint32_t ToAlbedo(const uint8_t* palette, int val)
{
    int c = val;
    c = c < 0 ? 0 : ((c >> 1) & 0x38);
    c = palette[c] & 0xF;

    uint32_t argb = colortable[c & 0xf] | 0xFF000000;
    uint32_t abgr = ((argb & 0xFF000000)) | // Keep alpha as is
                    ((argb & 0xFF) << 16) | // Move red to third position
                    ((argb & 0xFF00)) | // Keep green as is
                    ((argb & 0xFF0000) >> 16); // Move blue to rightmost
    return abgr;
}

PlanetSurface FractalGenerator::GetPlanetSurface(uint16_t planetInstanceIndex)
{

    auto planet = planets.at(planetInstanceIndex);
    uint16_t seed = planet.seed;

    PlanetSurface ps{};
    ps.relief.resize(48 * 24);
    ps.albedo.resize(48 * 24);
    ps.native = nativeImages.at(seed);

    const uint8_t* palette = GetPlanetColorMap(planet.species);

    std::vector<unsigned char> mini_png;
    if (planet.species == 18)
    {
        unsigned mini_width, mini_height;
        unsigned mini_error = lodepng::decode(mini_png, mini_width, mini_height, "mini_earth.png", LCT_GREY, 8);
        if (mini_error) {
            fprintf(stderr, "Error decoding mini PNG: %u: %s\n", mini_error, lodepng_error_text(mini_error));
        }
    }

    uint32_t t = 0;
    for (int j = 23; j >= 0; j--)
    {
        for (int i = 0; i < 48; i++)
        {
            int val = ps.native[t];
            ps.relief[t] = val + 128;
            ps.albedo[t] = ToAlbedo(palette, val);
            ++t;
        }
    }

    return ps;
}

#include "vstrace.h"

FullResPlanetData FractalGenerator::GetFullResPlanetData(uint16_t planetInstanceIndex)
{
    if (planetInstanceIndex == 0x10ad) {
        std::vector<unsigned char> image;
        unsigned MapWidth, MapHeight;
        unsigned error = lodepng::decode(image, MapWidth, MapHeight, "lofi_earth.png", LCT_GREY, 8);
        if (error) {
            std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
            return FullResPlanetData{};
        }

        if (MapWidth != (planet_usable_width * planet_contour_width) || MapHeight != (planet_usable_height * planet_contour_height)) {
            printf("Error: Map dimensions are not sane. Expected %dx%d but got %dx%d\n", 
                   planet_usable_width * planet_contour_width, planet_usable_height * planet_contour_height, 
                   MapWidth, MapHeight);
            return FullResPlanetData{};
        }

#if 0
        auto convertToHeightValue = [](unsigned char val) {

            uint16_t height_val = val /= 2;
            
            return (unsigned char)height_val;
        };

        std::transform(image.begin(), image.end(), image.begin(), convertToHeightValue);
#endif

        FullResPlanetData earthData;
        earthData.image = std::move(image);
        return earthData;
    }

    uint16_t seed = planets.at(planetInstanceIndex).seed;

    std::vector<unsigned char> localMemory(SystemMemorySize);

    MemoryScope memoryScope(localMemory.data());

    FullResPlanetData result{};

    auto native = nativeImages.find(seed);
    if (native == nativeImages.end())
    {
        return FullResPlanetData{};
    }

    Write16(cc_FNULL, 0xff80);

    // Emulates the setup done in MERCATOR-GEN

    Write16(pp_SEED, seed);
    Write16(pp_GLOBALSEED, seed);

    Push(pp_XYANCHOR);
    OFF();

    Push(MERCATOR_RATIO_1);
    Push(MERCATOR_RATIO_2);
    Push(MERCATOR_SCALE);
    SETSCALE();

    Push(pp_SPHEREWRAP);
    OFF();
    Push(pp_SIGNEXTEND);
    OFF();

    auto InitArray = [](uint16_t arrayDescriptor, const ArrayType& array) {
        Write16(arrayDescriptor, array.width);
        Write16(arrayDescriptor + 2, array.height);
        Write16(arrayDescriptor + 4, array.rowTable);
        Write16(arrayDescriptor + 6, array.segment);

        for (int i = 0; i < array.height; i++)
        {
            Write16Long(array.segment, array.rowTable + (i * 2), i * array.width);
        }
    };

    InitArray(MERCATOR_ARRAY, MERCATOR);
    InitArray(CONANCHOR_ARRAY, CONANCHOR);
    InitArray(CONTOUR_ARRAY, CONTOUR);

    Write16(0x4cf1, MERCATOR_ARRAY);

    static constexpr uint32_t arrayDescOffset = ComputeAddress(StarflightBaseSegment, 0x4cf1);

    const uint16_t arrayDesc = *reinterpret_cast<const uint16_t*>(&currentMemory[arrayDescOffset]);
    uint32_t arrayDescAddress = ComputeAddress(StarflightBaseSegment, arrayDesc);
    GlobalArrayDescriptor = reinterpret_cast<ArrayType*>(&currentMemory[arrayDescAddress]);    

    const uint16_t seg = 0x7e51;

    // Emulates the call to FRACT-REGION 

    uint32_t t = 0;
    for (int j = 23; j >= 0; j--)
    {
        for (int i = 0; i < 48; i++)
        {
            Write8Long(seg, j * 48 + i, native->second[t]);
            ++t;
        }
    }

    const int16_t xscale = 61;
    const int16_t yscale = 101;


    // One to throw away. No idea why
    {
        Write16(0x5916, 0);
        Write16(0x5921, 0);

        FRACT_NEWCONTOUR();
    }

    auto start = std::chrono::high_resolution_clock::now();

    auto userMarkRange = UserMarks::getInstance().createUserMarkRange("NEWCONTOUR");

    for (int16_t ycon = 0; ycon < planet_usable_height * yscale; ycon += yscale)
    {
        for (int16_t xcon = 0; xcon < planet_usable_width * xscale; xcon += xscale)
        {
            Write16(0x5916, xcon);
            Write16(0x5921, ycon);

            FRACT_NEWCONTOUR();

            uint16_t segment = 0x7cbe; // NEWCONTOUR segment

            for (int y = 0; y < planet_contour_height; ++y)
            {
                for (int x = 0; x < planet_contour_width; ++x)
                {
                    uint8_t val = static_cast<uint8_t>(Read8Long(segment, y * planet_contour_width + x));

                    int image_x = ((xcon / xscale) * planet_contour_width) + x;
                    int image_y = (planet_usable_height * yscale - 1) - (((ycon / yscale) * planet_contour_height) + y);
                    int image_index = image_y * (planet_contour_width * planet_usable_width) + image_x;
                    result.image[image_index] = val;
                    //planet_albedo[image_index] = ToAlbedo(palette, val);
                }
            }

        }
    }

    userMarkRange.reset();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    float milliseconds = duration.count() / 1000.0f;
    char buf[256];
    snprintf(buf, sizeof(buf), "NEWCONTOUR took %.3f milliseconds\n", milliseconds);
    OutputDebugStringA(buf);   

    #if 0
    std::vector<unsigned char> png;
    unsigned error = lodepng::encode(png, result.image, planet_contour_width * planet_usable_width, planet_contour_height * planet_usable_height, LCT_GREY, 8);
    if (!error)
    {
        std::string filename = "planets/" + std::to_string(seed) + ".png";
        lodepng::save_file(png, filename);
    }
    else
    {
        fprintf(stderr, "Error encoding PNG: %u: %s\n", error, lodepng_error_text(error));
    }
    #endif

    return result;
}

