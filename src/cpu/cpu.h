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

#define ComputeAddress(segment, offset) (((unsigned long)(segment) << 4) + (offset))
//unsigned long ComputeAddress(unsigned short segment, unsigned short offset);

void Push(unsigned short x);
unsigned short Pop();
#else

#define ComputeAddress(segment, offset) (((unsigned long)(segment) << 4) + (offset))

// Template functions for memory operations
template <typename Memory>
inline void Write8Long(Memory& memory, unsigned short s, unsigned short o, unsigned char x) {
    unsigned long addr = ComputeAddress(s, o);
    memory[addr] = x;
}

template <typename Memory>
inline void Write8(Memory& memory, unsigned short offset, unsigned char x) {
    Write8Long(memory, StarflightBaseSegment, offset, x);
}

template <typename Memory>
inline void Write16(Memory& memory, unsigned short offset, uint16_t x) {
    uint16_t* addr = reinterpret_cast<uint16_t*>(&memory[ComputeAddress(StarflightBaseSegment, offset)]);
    *addr = x;
}

template <typename Memory>
inline void Write16Long(Memory& memory, unsigned short s, unsigned short o, uint16_t x) {
    uint16_t* addr = reinterpret_cast<uint16_t*>(&memory[ComputeAddress(s, o)]);
    *addr = x;
}

template <typename Memory>
inline unsigned char Read8(Memory& memory, unsigned short offset) {
    return memory[ComputeAddress(StarflightBaseSegment, offset)];
}

template <typename Memory>
inline unsigned char Read8Long(Memory& memory, unsigned short s, unsigned short o) {
    return memory[ComputeAddress(s, o)];
}

template <typename Memory>
inline uint16_t Read16(Memory& memory, unsigned short offset) {
    return *reinterpret_cast<uint16_t*>(&memory[ComputeAddress(StarflightBaseSegment, offset)]);
}

template <typename Memory>
inline uint16_t Read16Long(Memory& memory, unsigned short s, unsigned short o) {
    return *reinterpret_cast<uint16_t*>(&memory[ComputeAddress(s, o)]);
}

// Default functions using the 'm' memory array
inline void Write8Long(unsigned short s, unsigned short o, unsigned char x) {
    Write8Long(m, s, o, x);
}

inline void Write8(unsigned short offset, unsigned char x) {
    Write8(m, offset, x);
}

inline void Write16(unsigned short offset, uint16_t x) {
    Write16(m, offset, x);
}

inline void Write16Long(unsigned short s, unsigned short o, uint16_t x) {
    Write16Long(m, s, o, x);
}

inline unsigned char Read8(unsigned short offset) {
    return Read8(m, offset);
}

inline unsigned char Read8Long(unsigned short s, unsigned short o) {
    return Read8Long(m, s, o);
}

inline uint16_t Read16(unsigned short offset) {
    return Read16(m, offset);
}

inline uint16_t Read16Long(unsigned short s, unsigned short o) {
    return Read16Long(m, s, o);
}

#define Push(x) \
    [](uint16_t val){ \
        regsp -= 2; \
        Write16(regsp, val); \
    }(x)

#define Pop() \
    [](){ \
        uint16_t x = Read16(regsp); \
        regsp += 2; \
        return x; \
    }()

#endif // USE_INLINE_MEMORY

void InitCPU();

// Actual 8086 emulator, exposed in 8086emu.cpp
void Init8086(uint8_t* systemMemory);
void Run8086(uint16_t cs, uint16_t ip, uint16_t ds, uint16_t ss, uint16_t *regSp);
unsigned disassemble(unsigned seg, unsigned off, uint8_t *memory, int count);

#endif
