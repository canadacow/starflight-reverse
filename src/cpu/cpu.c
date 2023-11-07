#include "cpu.h"
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include "../emul/callstack.h"

unsigned char m[1024*1024];
unsigned char *mem;
unsigned short regsp;
unsigned short regbp;
unsigned short int regsi = 0x129; // current vocabulary address (the forth pc pointer)

unsigned short regbx;

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

void Write8(unsigned short offset, unsigned char x)
{
    mem[offset] = x;
}

void Write8Long(unsigned short s, unsigned short o, unsigned char x)
{
    unsigned long addr = ComputeAddress(s, o);

    m[addr] = x;
}

void Write16(unsigned short offset, unsigned short x)
{
    mem[offset+0] = (x>>0)&0xFF;
    mem[offset+1] = (x>>8)&0xFF;
}

void Write16Long(unsigned short s, unsigned short o, unsigned short x)
{
    unsigned long addr = ComputeAddress(s, o);
 
    m[addr+0] = x&0xFF;
    m[addr+1] = x>>8;
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
    memset(m, 0, 1024*1024);
    mem = &m[0x192 << 4];
}


