#ifndef CALL_H
#define CALL_H

#include <string>
#include <functional>
#include <filesystem>

enum RETURNCODE {OK, ERROR, EXIT, CHARINPUT, STOP};

enum RETURNCODE Call(unsigned short addr, unsigned short bx);
enum RETURNCODE Step();
void InitEmulator(std::filesystem::path path);
void SaveSTARFLT();
void EnableDebug();
void PrintCStack();

void FillKeyboardBufferString(const char *str);
void FillKeyboardBufferKey(unsigned short key);

#endif
