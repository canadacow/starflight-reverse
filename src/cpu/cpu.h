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

#define USE_MACRO_COMPUTE_ADDRESS 1
#define USE_MACRO_WRITE8LONG 1
#define USE_MACRO_WRITE8 1
#define USE_MACRO_WRITE16 1
#define USE_MACRO_WRITE16LONG 1
#define USE_MACRO_READ8 1
#define USE_MACRO_READ8LONG 1
#define USE_MACRO_READ16 1
#define USE_MACRO_READ16LONG 1
#define USE_MACRO_PUSH 1
#define USE_MACRO_POP 1

#if USE_MACRO_COMPUTE_ADDRESS
#define ComputeAddress(segment, offset) (((unsigned long)(segment) << 4) + (offset))
#else
unsigned long ComputeAddress(unsigned short segment, unsigned short offset);
#endif

#if USE_MACRO_WRITE8LONG
#define Write8Long(s, o, x) do { \
    unsigned long addr = ComputeAddress(s, o); \
    m[addr] = x; \
} while(0)
#else
void Write8Long(unsigned short s, unsigned short o, unsigned char x);
#endif

#if USE_MACRO_WRITE8
#define Write8(offset, x) Write8Long(StarflightBaseSegment, offset, x)
#else
void Write8(unsigned short offset, unsigned char x);
#endif

#if USE_MACRO_WRITE16
#define Write16(offset, x) do { \
    uint16_t* addr = reinterpret_cast<uint16_t*>(&m[ComputeAddress(StarflightBaseSegment, offset)]); \
    *addr = (x); \
} while(0)
#else
void Write16(unsigned short offset, uint16_t x);
#endif

#if USE_MACRO_WRITE16LONG
#define Write16Long(s, o, x) do { \
    uint16_t* addr = reinterpret_cast<uint16_t*>(&m[ComputeAddress(s, o)]); \
    *addr = (x); \
} while(0)
#else
void Write16Long(unsigned short s, unsigned short o, uint16_t x);
#endif

#if USE_MACRO_READ8
#define Read8(offset) (mem[offset])
#else
unsigned char Read8(unsigned short offset);
#endif

#if USE_MACRO_READ8LONG
#define Read8Long(s, o) (m[ComputeAddress(s, o)])
#else
unsigned char Read8Long(unsigned short s, unsigned short o);
#endif

#if USE_MACRO_READ16
#define Read16(offset) (*reinterpret_cast<uint16_t*>(&mem[offset]))
#else
uint16_t Read16(unsigned short offset);
#endif

#if USE_MACRO_READ16LONG
#define Read16Long(s, o) (*reinterpret_cast<uint16_t*>(&m[ComputeAddress(s, o)]))
#else
uint16_t Read16Long(unsigned short s, unsigned short o);
#endif

#if USE_MACRO_PUSH
#define Push(x) do { \
    regsp -= 2; \
    Write16(regsp, x); \
} while(0)
#else
void Push(uint16_t x);
#endif

#if USE_MACRO_POP
#define Pop() \
    [](){ \
        uint16_t x = Read16(regsp); \
        regsp += 2; \
        return x; \
    }()
#else
uint16_t Pop();
#endif

#endif // USE_INLINE_MEMORY

void InitCPU();

// Actual 8086 emulator, exposed in 8086emu.cpp
void Init8086(uint8_t* systemMemory);
void Run8086(uint16_t cs, uint16_t ip, uint16_t ds, uint16_t ss, uint16_t *regSp);
unsigned disassemble(unsigned seg, unsigned off, uint8_t *memory, int count);

#endif
