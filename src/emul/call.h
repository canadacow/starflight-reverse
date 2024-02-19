#ifndef CALL_H
#define CALL_H

#include <string>
#include <functional>

enum RETURNCODE {OK, ERROR, EXIT, INPUT, STOP};

enum RETURNCODE Call(unsigned short addr, unsigned short bx);
enum RETURNCODE Step();
void InitEmulator(std::string hash = "");
void SaveSTARFLT();
void EnableDebug();
void PrintCStack();

void FillKeyboardBufferString(const char *str);
void FillKeyboardBufferKey(unsigned short key);

#endif
