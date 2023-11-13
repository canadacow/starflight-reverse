#ifndef CPU_H
#define CPU_H

#include <stdint.h>

constexpr uint16_t StarflightBaseSegment = 0x192;

extern unsigned char *mem;
extern unsigned char m[1024*1024];
extern unsigned short regsp;
extern unsigned short regbp;
extern unsigned short regsi;

void Write8(unsigned short offset, unsigned char x);
void Write8Long(unsigned short s, unsigned short o, unsigned char x);
void Write16(unsigned short offset, unsigned short x);
void Write16Long(unsigned short s, unsigned short o, unsigned short x);
unsigned char Read8(unsigned short offset);
unsigned char Read8Long(unsigned short s, unsigned short o);
unsigned short Read16(unsigned short offset);
unsigned short Read16Long(unsigned short s, unsigned short o);

unsigned long ComputeAddress(unsigned short segment, unsigned short offset);

void Push(unsigned short x);
unsigned short Pop();

void InitCPU();

#endif
