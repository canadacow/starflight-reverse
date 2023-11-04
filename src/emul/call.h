#ifndef CALL_H
#define CALL_H

#include <string>
#include <functional>

using PollForInputType = std::function<bool(uint16_t* key)>;

enum RETURNCODE {OK, ERROR, EXIT, INPUT};

enum RETURNCODE Call(unsigned short addr, unsigned short bx, PollForInputType pollForInput);
enum RETURNCODE Step(PollForInputType pollForInput);
void InitEmulator(std::string hash = "");
void SaveSTARFLT();
void EnableDebug();
void PrintCStack();

void FillKeyboardBufferString(const char *str);
void FillKeyboardBufferKey(unsigned short key);

#endif
