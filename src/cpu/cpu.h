#ifndef CPU_H
#define CPU_H

#include <stdint.h>

constexpr uint32_t StarflightBaseSegment = 0x192;
constexpr uint32_t SystemMemorySize = 0x10FFF0;

extern unsigned char *mem;
extern unsigned char m[SystemMemorySize];
extern unsigned short regsp;
extern unsigned short regbp;
extern unsigned short regsi;

#if !defined(USE_INLINE_MEMORY)
void Write8(unsigned short offset, unsigned char x);
void Write8Long(unsigned short s, unsigned short o, unsigned char x);
void Write16(unsigned short offset, unsigned short x);
void Write16Long(unsigned short s, unsigned short o, unsigned short x);
unsigned char Read8(unsigned short offset);
unsigned char Read8Long(unsigned short s, unsigned short o);
unsigned short Read16(unsigned short offset);
unsigned short Read16Long(unsigned short s, unsigned short o);

unsigned char* Read8Addr(unsigned short offset);

unsigned long ComputeAddress(unsigned short segment, unsigned short offset);

void Push(unsigned short x);
unsigned short Pop();
#else
static inline unsigned long ComputeAddress(unsigned short segment, unsigned short offset)
{
    unsigned long addr = ((unsigned long)segment << 4) + offset;
    return addr;
}

static inline void Write8Long(unsigned short s, unsigned short o, unsigned char x)
{
    unsigned long addr = ComputeAddress(s, o);

    m[addr] = x;
}

static inline void Write8(unsigned short offset, unsigned char x)
{
    Write8Long(StarflightBaseSegment, offset, x);
}

static inline void Write16(unsigned short offset, unsigned short x)
{
    Write8Long(StarflightBaseSegment, offset, (x>>0)&0xFF);
    Write8Long(StarflightBaseSegment, offset+1, (x>>8)&0xFF);
}

static inline void Write16Long(unsigned short s, unsigned short o, unsigned short x)
{
    Write8Long(s, o, x&0xFF);
    Write8Long(s, o +1, x>>8);
}

static inline unsigned char Read8(unsigned short offset)
{
    return mem[offset];
}

static inline unsigned char Read8Long(unsigned short s, unsigned short o)
{
    unsigned long addr = ComputeAddress(s, o);

    return m[addr];
}

static inline unsigned char* Read8Addr(unsigned short offset)
{
    return &mem[offset];
}


static inline unsigned short Read16(unsigned short offset)
{
    unsigned short val = mem[offset+0] | (mem[offset+1]<<8);

    return val;
}

static inline unsigned short Read16Long(unsigned short s, unsigned short o)
{
    unsigned long addr = ComputeAddress(s, o);

    return m[addr + 0] | (m[addr + 1]<<8);
}

static inline void Push(unsigned short x)
{
    regsp -= 2;
    Write16(regsp, x);
}

static inline unsigned short Pop()
{
    unsigned short x = Read16(regsp);
    regsp += 2;
    return x;
}
#endif

void InitCPU();

// Actual 8086 emulator, exposed in 8086emu.cpp
void Init8086(uint8_t* systemMemory);
void Run8086(uint16_t cs, uint16_t ip, uint16_t ds, uint16_t ss, uint16_t *regSp);
unsigned disassemble(unsigned seg, unsigned off, uint8_t *memory, int count);

#endif
