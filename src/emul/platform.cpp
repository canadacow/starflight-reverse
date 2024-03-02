#include "platform.h"

#include <thread>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

void SetCurrentThreadName(std::string threadName)
{
    #ifdef _WIN32
    // Convert threadName to wide string for Windows API
    std::wstring wThreadName(threadName.begin(), threadName.end());
    SetThreadDescription(GetCurrentThread(), wThreadName.c_str());
    #elif defined(__linux__)
    pthread_setname_np(pthread_self(), threadName.c_str());
    #elif defined(__APPLE__)
    pthread_setname_np(threadName.c_str());
    #endif
}