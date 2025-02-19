#include "cpu.h"
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include "../emul/callstack.h"

unsigned char m[SystemMemorySize];
unsigned char *mem;
unsigned short regsp;
unsigned short regbp;
unsigned short int regsi; // current vocabulary address (the forth pc pointer)

unsigned short regbx;

#if 0
unsigned long ComputeAddress(unsigned short segment, unsigned short offset)
{
    unsigned long addr = ((unsigned long)segment << 4) + offset;

    // Nebula detection code (to see if the ship is actually in the nebula)
    // reads directly from video memory

    if(addr >= 0xA0000 && addr <= 0xAFFFF)
    {
        printf("Access to video memory from program itself.\n");
        PrintCallstacktrace(regbx);
        assert(0);
    }
    return addr;
}
#endif

void Write8(unsigned short offset, unsigned char x)
{
    Write8Long(StarflightBaseSegment, offset, x);
}

void Write8Long(unsigned short s, unsigned short o, unsigned char x)
{
    unsigned long addr = ComputeAddress(s, o);

    m[addr] = x;

    #if 0
    if(addr == 0x7d20)
    {
        printf("Hull at %d\n", m[addr]);
        fflush(stdout);
    }
    #endif

    #if 0
    if (addr == ComputeAddress(StarflightBaseSegment, 0x5dae))
    {
        printf("");
    }
    #endif
    
    #if 0
    constexpr uint16_t hullvalue = 0x63ef + 0x11;

    if((s == StarflightBaseSegment) && ((o == hullvalue) || o == (hullvalue +1)))
    {
        uint16_t hull = Read16(0x63ef + 0x11);
        //if(hull != 100)
        {
            printf("Hull at %d\n", hull);
            fflush(stdout);
        }
    }
    #endif
}

void Write16(unsigned short offset, unsigned short x)
{
    Write8Long(StarflightBaseSegment, offset, (x>>0)&0xFF);
    Write8Long(StarflightBaseSegment, offset+1, (x>>8)&0xFF);
}

void Write16Long(unsigned short s, unsigned short o, unsigned short x)
{
    Write8Long(s, o, x&0xFF);
    Write8Long(s, o +1, x>>8);
}

unsigned char Read8(unsigned short offset)
{
    return mem[offset];
}

unsigned char Read8Long(unsigned short s, unsigned short o)
{
    unsigned long addr = ComputeAddress(s, o);

    return m[addr];
}

unsigned char* Read8Addr(unsigned short offset)
{
    return &mem[offset];
}


unsigned short Read16(unsigned short offset)
{
    unsigned short val = mem[offset+0] | (mem[offset+1]<<8);

    return val;
}

unsigned short Read16Long(unsigned short s, unsigned short o)
{
    unsigned long addr = ComputeAddress(s, o);

    return m[addr + 0] | (m[addr + 1]<<8);
}

void Push(unsigned short x)
{
    regsp -= 2;
    Write16(regsp, x);
}

unsigned short Pop()
{
    unsigned short x = Read16(regsp);
    regsp += 2;
    return x;
}

void InitCPU()
{
    regsi = 0x129;
    memset(m, 0, SystemMemorySize);
    mem = &m[StarflightBaseSegment << 4];
    Init8086(m);
}


