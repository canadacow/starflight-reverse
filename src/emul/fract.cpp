#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#ifndef DEBUG
#define USE_INLINE_MEMORY
#endif

#include "fract.h"

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
const unsigned short CONTOUR_RATIO_1 = 20882;
const unsigned short CONTOUR_RATIO_2 = 32767;

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
    
    // Array and wrapping control
    uint16_t xy_anchor;       // 0xe356 XYANCHOR
    uint16_t sphere_wrap;     // 0x4cca SPHEREWRAP
    uint16_t sign_extend;     // 0x4cd8 SIGNEXTEND
    
    // Original coordinate storage
    uint16_t x0;             // 0xe370 X0'
    uint16_t y0;             // 0xe374 Y0'
};

ArrayType CONANCHOR = {0x0009, 0x0007, 0x003f, 0x938c};
ArrayType CONTOUR = {0x003d, 0x0065, 0x1811, 0x91fe};

ArrayType* GlobalArrayDescriptor = nullptr;
static constexpr uint32_t seedOffset = ComputeAddress(StarflightBaseSegment, 0x4ab0);
static uint16_t* RandomSeed = reinterpret_cast<uint16_t*>(&m[seedOffset]);

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

  const uint16_t arrayDesc = *reinterpret_cast<const uint16_t*>(&m[arrayDescOffset]);
  uint32_t arrayDescAddress = ComputeAddress(StarflightBaseSegment, arrayDesc);
  GlobalArrayDescriptor = reinterpret_cast<ArrayType*>(&m[arrayDescAddress]);

  Push(0x4d5c); // 'W4D5C'
  Push(0x4cdc); // W4CDC
  Store(); // !
  Push(0x2ee5); // 'LC!'
  Push(0x4ce0); // W4CE0
  Store(); // !
  Push(Read16(0x4cd8)); // SIGNEXTEND @
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

FORCE_INLINE void C_PLUS_LIMIT()
{
    signed short bx, ax, cx;
    ax = Pop();
    cx = Pop();
    ax += cx;
    if (ax > 0x7F)
    {
        ax = 0x7F;
    }
    if (ax < -0x7F)
    {
        ax = -0x7F;
    }
    Push(ax);
}

FORCE_INLINE void DISPLACEMENT(FractalState& fractalState)
{
    unsigned short ax, cx, dx;
    ax = Pop();
    cx = Pop();
    dx = Read16(0xe356); // XYANCHOR
    if (dx)
    {
        cx += Read16(pp_X0_i_); // X0'
        ax += Read16(pp_Y0_i_); // Y0'
        if (ax&0x8000)
        {
            ax = -(ax + 1);
            cx += 0x0480;
        }
        else
        {
            dx = 0x3c0;
            if (dx <= ax)
            {
                ax = dx - ax + dx + 1;
                cx += 0x480;
            }
        }
        *RandomSeed = ax; // SEED
        Push(cx);

        ax = 1;
        Push(ax);
        ax = -1;
        Push(ax);
        RRND();

        cx = Pop();
        *RandomSeed = Pop(); // SEED
        Push(cx);

        ax = 1;
        Push(ax);
        ax = -1;
        Push(ax);
        RRND();

        ax = Pop();
        cx = Pop();
        ax = ax ^ Read16(0x5979); // GLOBALSEED
        ax = ax ^ cx;
        *RandomSeed = ax; // SEED
    }

    ax = fractalState.std;
    cx = -ax;
    Push(cx);
    Push(ax);
    RRND();
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
    return Read16(0x4cca) != 0; // SPHEREWRAP
}

FORCE_INLINE bool IsSignExtend() {
    return Read16(0x4cd8) != 0; // SIGNEXTEND  
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

FORCE_INLINE void FRACT_StoreHeight() // Set Anchor
{
    unsigned short ax;
    uint16_t segment, offset;
    ACELLADDR_TO_SEG_OFF(segment, offset); // Get segment and offset
    ax = Read8Long(segment, offset); // Read value using segment and offset
    if (ax == 0x80) // not set yet
    {
        ax = Pop();
        Write8Long(segment, offset, ax & 0xFF); // Write value using segment and offset
    }
    else
    {
        ax = Pop();
    }
}

void Ext_FRACT_StoreHeight() {

    if(GlobalArrayDescriptor == nullptr) {
      static constexpr uint32_t arrayDescOffset = ComputeAddress(StarflightBaseSegment, 0x4cf1);

      const uint16_t arrayDesc = *reinterpret_cast<const uint16_t*>(&m[arrayDescOffset]);
      uint32_t arrayDescAddress = ComputeAddress(StarflightBaseSegment, arrayDesc);
      GlobalArrayDescriptor = reinterpret_cast<ArrayType*>(&m[arrayDescAddress]);
    }

    FRACT_StoreHeight();
}

FORCE_INLINE void XSHIFT(FractalState& fractalState)
{
    unsigned short ax, bx, cx;
    fractalState.temp_y = Pop(); // TY

    Push(fractalState.x_lower_left);
    Push(fractalState.temp_y); // TY
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    Push(fractalState.x_upper_right);
    Push(fractalState.temp_y); // TY
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    ax = Pop();

    cx = Pop();
    ax += cx;
    ax = ((signed short)ax) >> 1;
    Push(ax);

    bx = fractalState.x_lower_left;
    bx += fractalState.x_upper_right;
    bx = ((signed short)bx) >> 1;
    Push(bx);

    Push(fractalState.temp_y); // TY

    DISPLACEMENT(fractalState);
    C_PLUS_LIMIT();

    Push(fractalState.x_mid);
    Push(fractalState.temp_y); // TY
    FRACT_StoreHeight();
}

FORCE_INLINE void YSHIFT(FractalState& fractalState)
{
    unsigned short ax, bx, cx;

    fractalState.temp_y = Pop(); // TY

    Push(fractalState.temp_y); // TY
    Push(fractalState.y_lower_left);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    Push(fractalState.temp_y); // TY
    Push(fractalState.y_upper_right);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();
    ax = Pop();

    cx = Pop();
    ax += cx;
    ax = ((signed short)ax) >> 1;
    Push(ax);

    Push(fractalState.temp_y); // TY
    bx = fractalState.y_lower_left;
    bx += fractalState.y_upper_right;
    bx = ((signed short)bx) >> 1;
    Push(bx);

    DISPLACEMENT(fractalState);
    C_PLUS_LIMIT();

    Push(fractalState.temp_y); // TY
    Push(fractalState.y_mid);
    FRACT_StoreHeight();
}

FORCE_INLINE void EDGES(FractalState& fractalState)
{
    unsigned short ax;
    ax = fractalState.dy_greater_than_one; // DY>1
    if (ax != 0)
    {
        ax = fractalState.x_lower_left;
        if (ax == 0) {
            Push(ax);
            YSHIFT(fractalState);
        }
        Push(fractalState.x_upper_right);
        YSHIFT(fractalState);
    }

    ax = fractalState.dx_greater_than_one; // DX>1
    if (ax != 0)
    {
        ax = fractalState.y_lower_left;
        if (ax == 0)
        {
            Push(ax);
            XSHIFT(fractalState);
        }
        Push(fractalState.y_upper_right);
        XSHIFT(fractalState);
    }
}

FORCE_INLINE void CENTER(FractalState& fractalState)
{
    unsigned short ax, cx;
    ax = fractalState.dy_greater_than_one & fractalState.dx_greater_than_one; // DY>1 and DX>1
    if (ax == 0) return;

    Push(fractalState.x_mid);
    Push(fractalState.y_lower_left);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();

    Push(fractalState.x_mid);
    Push(fractalState.y_upper_right);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();

    Push(fractalState.x_lower_left);
    Push(fractalState.y_mid);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();

    Push(fractalState.x_upper_right);
    Push(fractalState.y_mid);
    //ACELLADDR();
    //AGet();
    ACELLADDR_AND_GET();

    // calculate average
    ax = Pop() + Pop() + Pop() + Pop();
    ax = ((signed short)ax) >> 2;
    Push(ax);

    Push(fractalState.x_mid);
    Push(fractalState.y_mid);
    DISPLACEMENT(fractalState);
    C_PLUS_LIMIT();

    Push(fractalState.x_mid);
    Push(fractalState.y_mid);
    FRACT_StoreHeight();
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

void NEWSTD(FractalState& fractalState)
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
    unsigned short ax, cx;

    // Calculate DX>1 and DY>1
    ax = (fractalState.x_upper_right - fractalState.x_lower_left - 1) > 0 ? 1 : 0;
    fractalState.dx_greater_than_one = ax;

    cx = (fractalState.y_upper_right - fractalState.y_lower_left - 1) > 0 ? 1 : 0;
    fractalState.dy_greater_than_one = cx;

    if (cx > 0 || ax > 0)
    {
        MIDPT(fractalState);
        EDGES(fractalState);
        CENTER(fractalState);
        NEWSTD(fractalState);

        // Recursive calls with different parameters
        for (int i = 0; i < 4; ++i) {
            ax = 0;

            FractalState newfractalState = fractalState;

            switch (i) {
                case 0:
                    newfractalState.x_lower_left  = fractalState.x_lower_left;  // Push(Read16(regbp + 0x8));
                    newfractalState.y_lower_left  = fractalState.y_lower_left;  // Push(Read16(regbp + 0x6));
                    newfractalState.x_upper_right = fractalState.x_mid;         // Push(Read16(regbp + 0xC));
                    newfractalState.y_upper_right = fractalState.y_mid;         // Push(Read16(regbp + 0xA));
                    break;
                case 1:
                    newfractalState.x_lower_left  = fractalState.x_mid;         // Push(Read16(regbp + 0xC));
                    newfractalState.y_lower_left  = fractalState.y_lower_left;  // Push(Read16(regbp + 0x6));
                    newfractalState.x_upper_right = fractalState.x_upper_right; // Push(Read16(regbp + 0x4));
                    newfractalState.y_upper_right = fractalState.y_mid;         // Push(Read16(regbp + 0xA));
                    break;
                case 2:
                    newfractalState.x_lower_left  = fractalState.x_lower_left;  // Push(Read16(regbp + 0x8));
                    newfractalState.y_lower_left  = fractalState.y_mid;         // Push(Read16(regbp + 0xA));
                    newfractalState.x_upper_right = fractalState.x_mid;         // Push(Read16(regbp + 0xC));
                    newfractalState.y_upper_right = fractalState.y_upper_right; // Push(Read16(regbp + 0x2));
                    break;
                case 3:
                    newfractalState.x_lower_left  = fractalState.x_mid;         // Push(Read16(regbp + 0xC));
                    newfractalState.y_lower_left  = fractalState.y_mid;         // Push(Read16(regbp + 0xA));
                    newfractalState.x_upper_right = fractalState.x_upper_right; // Push(Read16(regbp + 0x4));
                    newfractalState.y_upper_right = fractalState.y_upper_right; // Push(Read16(regbp + 0x2));
                    break;
            }
            FRACTAL(newfractalState); // Recursive call
        }
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
  Push(Read16(cc_CONTOUR_dash_SCALE)); // CONTOUR-SCALE
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