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

#define ComputeAddress(segment, offset) (((unsigned long)(segment) << 4) + (offset))

#define Write8Long(s, o, x) do { \
    unsigned long addr = ComputeAddress(s, o); \
    m[addr] = x; \
} while(0)

#define Write8(offset, x) Write8Long(StarflightBaseSegment, offset, x)

#define Write16(offset, x) do { \
    uint16_t* addr = reinterpret_cast<uint16_t*>(&m[ComputeAddress(StarflightBaseSegment, offset)]); \
    *addr = (x); \
} while(0)

#define Write16Long(s, o, x) do { \
    uint16_t* addr = reinterpret_cast<uint16_t*>(&m[ComputeAddress(s, o)]); \
    *addr = (x); \
} while(0)

#define Read8(offset) (mem[offset])

static inline unsigned char* Read8Addr(unsigned short offset)
{
    return &mem[offset];
}

#define Read8Long(s, o) (m[ComputeAddress(s, o)])

#define Read16(offset) (*reinterpret_cast<uint16_t*>(&mem[offset]))

#define Read16Long(s, o) (*reinterpret_cast<uint16_t*>(&m[ComputeAddress(s, o)]))

#define Push(x) do { \
    regsp -= 2; \
    Write16(regsp, x); \
} while(0)

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
