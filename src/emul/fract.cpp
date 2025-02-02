#include<stdio.h>
#include<stdlib.h>

#ifndef DEBUG
#define USE_INLINE_MEMORY
#endif

#include "fract.h"

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

typedef struct
{
    unsigned short int width;
    unsigned short int height;
    unsigned short bx;
    unsigned short ds;
} ArrayType;

ArrayType CONANCHOR = {0x0009, 0x0007, 0x003f, 0x938c};
ArrayType CONTOUR = {0x003d, 0x0065, 0x1811, 0x91fe};

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
    unsigned short temp;
    unsigned short a = Pop(); // Pop the top element
    unsigned short b = Pop(); // Pop the second element
    temp = a;                 // Store the top element in a temporary variable
    a = b;                    // Move the second element to the top
    b = temp;                 // Move the temporary variable to the second position
    Push(a);                  // Push the new top element
    Push(b);                  // Push the new second element
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

void SETLARRAY() // SETLARRAY
{
  Push(0x4cf1); // 'ARRAY
  Store(); // !
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

void RRND()
{
    unsigned short ax, bx, cx, dx;
    ax = Read16(0x4ab0); // SEED
    cx = 0x7abd;
    ax = ((signed short)cx) * ((signed short)ax);
    ax += 0x1b0f;
    Write16(0x4ab0, ax); // SEED

    bx = Pop(); // range
    cx = Pop(); // low
    bx -= cx;
    unsigned int mul = ((unsigned int)ax) * ((unsigned int)bx);
    dx = ((mul >> 16)&0xFFFF) + cx;
    Push(dx);
}

void C_PLUS_LIMIT()
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

void DISPLACEMENT()
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
        Write16(0x4ab0, ax); // SEED
        Push(cx);

        ax = 1;
        Push(ax);
        ax = -1;
        Push(ax);
        RRND();

        cx = Pop();
        Write16(0x4ab0, Pop()); // SEED
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
        Write16(0x4ab0, ax); // SEED
    }

    ax = Read16(regbp+0xe);
    cx = -ax;
    Push(cx);
    Push(ax);
    RRND();
}

void SWRAP()
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

void ACELLADDR()
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

void AGet() // [A@]
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

void A_ex_() // [A!]
{
    unsigned short value = Pop();
    // Write to same memory locations that AGet reads from
    Write8Long(Read16(0xe378), Read16(0xe37c), (unsigned char)value); // SCELL OCELL
}

void AV_dash_MIDPT() // AV-MIDPT
{
    unsigned short int a, b, c;
    _2OVER(); // 2OVER
    ACELLADDR(); // ACELLADDR
    AGet(); // A@
    a = Pop(); // >R
    _2DUP(); // 2DUP
    ACELLADDR(); // ACELLADDR
    AGet(); // A@
    Push(Pop() + a >> 1); //  R> + 2/
    b = Pop(); // >R
    ROT(); // ROT
    Push(Pop() + Pop()); // +
    Push(Pop() >> 1); //  2/
    c = Pop(); // >R
    Push(Pop() + Pop()); // +
    Push(Pop() >> 1); //  2/
    Push(c); // R>
    Push(b); // R>
    ROT(); // ROT
    ROT(); // ROT
    ACELLADDR(); // ACELLADDR
    A_ex_(); // A!
}

void FRACT_StoreHeight() // Set Anchor
{
    unsigned short ax;
    ACELLADDR();
    AGet();
    ax = Pop();
    if (ax == 0xFF80) // not set yet
    {
        ax = Pop();
        Write8Long(Read16(0xe378), Read16(0xe37c), ax&0xFF); // SCELL OCELL
    }
    else
    {
        ax = Pop();
    }
}

void XSHIFT()
{
    unsigned short ax, bx, cx;
    Write16(0xe396, Pop()); // TY
    Push(Read16(regbp+8));
    Push(Read16(0xe396)); // TY
    ACELLADDR();
    AGet();
    Push(Read16(regbp+4));
    Push(Read16(0xe396)); // TY
    ACELLADDR();
    AGet();
    ax = Pop();

    cx = Pop();
    ax += cx;
    ax = ((signed short)ax) >> 1;
    Push(ax);

    bx = Read16(regbp+8);
    bx += Read16(regbp+4);
    bx = ((signed short)bx) >> 1;
    Push(bx);

    Push(Read16(0xe396)); // TY

    DISPLACEMENT();
    C_PLUS_LIMIT();

    Push(Read16(regbp+0xc));
    Push(Read16(0xe396)); // TY
    FRACT_StoreHeight();
}

void YSHIFT()
{
    unsigned short ax, bx, cx;
    Write16(0xe396, Pop()); // TY
    Push(Read16(0xe396)); // TY
    Push(Read16(regbp+6));
    ACELLADDR();
    AGet();
    Push(Read16(0xe396)); // TY
    Push(Read16(regbp+2));
    ACELLADDR();
    AGet();
    ax = Pop();

    cx = Pop();
    ax += cx;
    ax = ((signed short)ax) >> 1;
    Push(ax);

    Push(Read16(0xe396)); // TY
    bx = Read16(regbp+6);
    bx += Read16(regbp+2);
    bx = ((signed short)bx) >> 1;
    Push(bx);

    DISPLACEMENT();
    C_PLUS_LIMIT();

    Push(Read16(0xe396)); // TY
    Push(Read16(regbp+0xa));
    FRACT_StoreHeight();
}

void EDGES()
{
    unsigned short ax;
    ax = Read16(0xe368); // DY>1
    if (ax != 0)
    {
        ax = Read16(regbp+8);
        if (ax == 0) {
            Push(ax);
            YSHIFT();
        }
        Push(Read16(regbp+4));
        YSHIFT();
    }

    ax = Read16(0xe36c); // DX>1
    if (ax != 0)
    {
        ax = Read16(regbp+6);
        if (ax == 0)
        {
            Push(ax);
            XSHIFT();
        }
        Push(Read16(regbp+2));
        XSHIFT();
    }
}

void CENTER()
{
    unsigned short ax, cx;
    ax = Read16(0xe368) & Read16(0xe36c); // DY>1 and DX>1
    if (ax == 0) return;

    Push(Read16(regbp+0xC));
    Push(Read16(regbp+0x6));
    ACELLADDR();
    AGet();

    Push(Read16(regbp+0xC));
    Push(Read16(regbp+0x2));
    ACELLADDR();
    AGet();

    Push(Read16(regbp+0x8));
    Push(Read16(regbp+0xA));
    ACELLADDR();
    AGet();

    Push(Read16(regbp+0x4));
    Push(Read16(regbp+0xA));
    ACELLADDR();
    AGet();

    // calculate average
    ax = Pop() + Pop() + Pop() + Pop();
    ax = ((signed short)ax) >> 2;
    Push(ax);

    Push(Read16(regbp+0xC));
    Push(Read16(regbp+0xA));
    DISPLACEMENT();
    C_PLUS_LIMIT();

    Push(Read16(regbp+0xC));
    Push(Read16(regbp+0xA));
    FRACT_StoreHeight();
}

void MIDPT()
{
    unsigned short ax;
    ax = Read16(regbp+8);
    ax += Read16(regbp+4);
    ax = ((signed short)ax) >> 1;
    Write16(regbp+0xC, ax);

    ax = Read16(regbp+6);
    ax += Read16(regbp+2);
    ax = ((signed short)ax) >> 1;
    Write16(regbp+0xA, ax);
}

void NEWSTD()
{
    unsigned int ax = Read16(regbp + 0xe);
    unsigned int ratio1 = Read16(0xe35e); // RATIO
    unsigned int ratio2 = Read16(0xe360); // RATIO
    ax = (ax * ratio2) / ratio1;
    if (((signed int)ax) <= 0) // not sure
    {
        ax = 1;
    }
    Write16(regbp+0xe, ax);
}

void FRACTAL()
{
    unsigned short ax, cx;
    regsp -= 2; // instruction pointer

    regbp = regsp;

    ax = Read16(regbp+4) - Read16(regbp+8) - 1;
    if ((signed short)ax <= 0) ax = 0; else ax = 1; // not sure
    Write16(0xe36c, ax); // DX>1

    cx = Read16(regbp+2) - Read16(regbp+6) - 1;
    if ((signed short)cx <= 0) cx = 0; else cx = 1; // not sure
    Write16(0xe368, cx); // DY>1

    if (cx > 0 || ax > 0)
    {
        MIDPT();
        EDGES();
        CENTER();
        NEWSTD();

        ax = 0;
        Push(Read16(regbp+0xe));
        Push(ax);
        Push(ax);
        Push(Read16(regbp+0x8));
        Push(Read16(regbp+0x6));
        Push(Read16(regbp+0xC));
        Push(Read16(regbp+0xA));
        FRACTAL(); // recursive

        ax = 0;
        Push(Read16(regbp+0xe));
        Push(ax);
        Push(ax);
        Push(Read16(regbp+0xC));
        Push(Read16(regbp+0x6));
        Push(Read16(regbp+0x4));
        Push(Read16(regbp+0xA));
        FRACTAL(); // recursive

        ax = 0;
        Push(Read16(regbp+0xe));
        Push(ax);
        Push(ax);
        Push(Read16(regbp+0x8));
        Push(Read16(regbp+0xA));
        Push(Read16(regbp+0xC));
        Push(Read16(regbp+0x2));
        FRACTAL(); // recursive

        ax = 0;
        Push(Read16(regbp+0xe));
        Push(ax);
        Push(ax);
        Push(Read16(regbp+0xC));
        Push(Read16(regbp+0xA));
        Push(Read16(regbp+0x4));
        Push(Read16(regbp+0x2));
        FRACTAL(); // recursive
    }

    ax = Pop();
    regsp += 0x0e; // 14
    regbp = regsp;
    Push(ax);
    regsp += 2; //instruction pointer
}

void FRACT_FRACTALIZE()
{
    unsigned short int yur, xur, yll, xll, dummy1, dummy2, std;
    yur = Read16(regsp+0);
    xur = Read16(regsp+2);
    yll = Read16(regsp+4);
    xll = Read16(regsp+6);
    dummy1 = Read16(regsp+8);
    dummy2 = Read16(regsp+10);
    std = Read16(regsp+12);
    //printf("FRACTALIZE xll=%i yll=%i xur=%i yur=%i std=%i\n", xll, yll, xur, yur, std);
    Write16(0xe392, regbp); // RTEMP
    FRACTAL();
    regbp = Read16(0xe392); // RTEMP
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

void MERC_gt_CONANCHOR() // MERC>CONANCHOR
{
  unsigned short int i, imax, j, jmax;
  Push(Read16(pp_YCON)); // YCON @
  Push(0x0028);
  _slash_(); // /
  Push(pp_Y2); // Y2
  Store_3(); // !_3
  Push(Read16(pp_XCON)); // XCON @
  Push(0x0030);
  _slash_(); // /
  Push(pp_X2); // X2
  Store_3(); // !_3
  Push(pp_SPHEREWRAP); // SPHEREWRAP
  ON_3(); // ON_3
  Push(0x6a99); // 'MERCATOR'
  SETLARRAY(); // SETLARRAY

  i = 0;
  imax = 4;
  do // (DO)
  {

    j = 0;
    jmax = 3;
    do // (DO)
    {
      Push(j + Read16(pp_X2)); // I X2 @ +
      Push(Read16(pp_Y2) + i); // Y2 @ J +
      ACELLADDR(); // ACELLADDR
      AGet(); // A@
      Push(j * 4); // I 4 *
      Push(i * 2); // J 2*
      ReadArray(CONANCHOR); // CONANCHOR
      LC_ex_(); // LC!
      j++;
    } while(j<jmax); // (LOOP)

    i++;
  } while(i<imax); // (LOOP)

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
      ReadArray(CONANCHOR); // CONANCHOR
      LC_at_(); // LC@
      Push(j * 0x000c); // I 0x000c *
      Push(i * 0x0014); // J 0x0014 *
      ReadArray(CONTOUR); // CONTOUR
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
  ACELLADDR(); // ACELLADDR
  AGet(); // A@
  Push(Pop()==Read16(cc_FNULL)?1:0); //  FNULL =
  a = Pop(); // >R
  OVER(); // OVER
  Push(Pop() + 0x000b); //  0x000b +
  OVER(); // OVER
  Push(Pop() + 0x0013); //  0x0013 +
  ACELLADDR(); // ACELLADDR
  AGet(); // A@
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
      Push(j); // I
      Push(i); // J
      SUB_dash_CON_dash_FRACT(); // SUB-CON-FRACT
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