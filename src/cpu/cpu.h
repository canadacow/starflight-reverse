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
extern unsigned short regbx;

#if 0 //!defined(USE_INLINE_MEMORY)
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

extern unsigned char* currentMemory;
static constexpr uint32_t seedOffset = ComputeAddress(StarflightBaseSegment, 0x4ab0);
extern uint16_t* RandomSeed;

#ifdef _WIN32
#ifndef DWORD
typedef unsigned long DWORD;
#endif

#define PAGE_NOACCESS 0x01
#define PAGE_READWRITE 0x02;
#define PAGE_WRITECOPY 0x03;
#define PAGE_EXECUTE_READ 0x20;
#define PAGE_EXECUTE_READWRITE 0x40;
#define PAGE_EXECUTE_WRITECOPY 0x80;

extern "C" __declspec(dllimport) int __stdcall VirtualProtect(void* lpAddress, size_t dwSize, DWORD flNewProtect, DWORD* lpflOldProtect);
#endif

struct CPUContext {
    unsigned short regsp;
    unsigned short regbp;
    unsigned short regsi;
    unsigned short regbx;
};

class MemoryScope {
    unsigned char* previous;
    CPUContext previousCPU;
#ifdef _WIN32
    DWORD previousProtect;
#endif
public:
    MemoryScope(unsigned char* mem) { 
        previousCPU = { regsp, regbp, regsi, regbx };

        previous = currentMemory;
        currentMemory = mem;    
        RandomSeed = reinterpret_cast<uint16_t*>(&currentMemory[seedOffset]);

        DWORD oldProtect;
        VirtualProtect(previous, SystemMemorySize, PAGE_NOACCESS, &oldProtect);
    }
    ~MemoryScope() { 
        regsp = previousCPU.regsp;
        regbp = previousCPU.regbp;
        regsi = previousCPU.regsi;
        regbx = previousCPU.regbx;

        currentMemory = previous;
        RandomSeed = reinterpret_cast<uint16_t*>(&currentMemory[seedOffset]);

        DWORD oldProtect;
        VirtualProtect(previous, SystemMemorySize, previousProtect, &oldProtect);
    }
};

// Template functions for memory operations
inline void Write8Long(unsigned char* memory, unsigned short s, unsigned short o, unsigned char x) {
    unsigned long addr = ComputeAddress(s, o);
    memory[addr] = x;
}

inline void Write8(unsigned char* memory, unsigned short offset, unsigned char x) {
    Write8Long(memory, StarflightBaseSegment, offset, x);
}

inline void Write16(unsigned char* memory, unsigned short offset, uint16_t x) {
    uint16_t* addr = reinterpret_cast<uint16_t*>(&memory[ComputeAddress(StarflightBaseSegment, offset)]);
    *addr = x;
}

inline void Write16Long(unsigned char* memory, unsigned short s, unsigned short o, uint16_t x) {
    uint16_t* addr = reinterpret_cast<uint16_t*>(&memory[ComputeAddress(s, o)]);
    *addr = x;
}

inline unsigned char Read8(unsigned char* memory, unsigned short offset) {
    return memory[ComputeAddress(StarflightBaseSegment, offset)];
}

inline unsigned char Read8Long(unsigned char* memory, unsigned short s, unsigned short o) {
    return memory[ComputeAddress(s, o)];
}

inline uint16_t Read16(unsigned char* memory, unsigned short offset) {
    return *reinterpret_cast<uint16_t*>(&memory[ComputeAddress(StarflightBaseSegment, offset)]);
}

inline uint16_t Read16Long(unsigned char* memory, unsigned short s, unsigned short o) {
    return *reinterpret_cast<uint16_t*>(&memory[ComputeAddress(s, o)]);
}

// Default functions using the 'm' memory array
inline void Write8Long(unsigned short s, unsigned short o, unsigned char x) {
    Write8Long(currentMemory, s, o, x);
}

inline void Write8(unsigned short offset, unsigned char x) {
    Write8(currentMemory, offset, x);
}

inline void Write16(unsigned short offset, uint16_t x) {
    Write16(currentMemory, offset, x);
}

inline void Write16Long(unsigned short s, unsigned short o, uint16_t x) {
    Write16Long(currentMemory, s, o, x);
}

inline unsigned char Read8(unsigned short offset) {
    return Read8(currentMemory, offset);
}

inline unsigned char Read8Long(unsigned short s, unsigned short o) {
    return Read8Long(currentMemory, s, o);
}

inline uint16_t Read16(unsigned short offset) {
    return Read16(currentMemory, offset);
}

inline uint16_t Read16Long(unsigned short s, unsigned short o) {
    return Read16Long(currentMemory, s, o);
}

inline unsigned char* Read8Addr(unsigned char* memory, unsigned short offset) {
    return &memory[ComputeAddress(StarflightBaseSegment, offset)];
}

inline unsigned char* Read8Addr(unsigned short offset) {
    return Read8Addr(currentMemory, offset);
}

inline void Push(uint16_t x) {
    regsp -= 2;
    Write16(regsp, x);
}

inline uint16_t Pop() {
    uint16_t x = Read16(regsp);
    regsp += 2;
    return x;
}

#endif // USE_INLINE_MEMORY

void InitCPU();

// Actual 8086 emulator, exposed in 8086emu.cpp
void Init8086(uint8_t* systemMemory);
void Run8086(uint16_t cs, uint16_t ip, uint16_t ds, uint16_t ss, uint16_t *regSp);
unsigned disassemble(unsigned seg, unsigned off, uint8_t *memory, int count);

#endif
