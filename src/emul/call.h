#ifndef CALL_H
#define CALL_H

#include <string>

enum RETURNCODE {OK, ERROR, EXIT, INPUT};

enum RETURNCODE Call(unsigned short addr, unsigned short bx);
enum RETURNCODE Step();
void InitEmulator(std::string hash = "");
void SaveSTARFLT();
void EnableDebug();
void PrintCStack();

void FillKeyboardBufferString(char *str);
void FillKeyboardBufferKey(unsigned short key);

#endif
