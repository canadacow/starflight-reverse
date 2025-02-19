#include "vstrace.h"

#ifdef _WIN32
#define DIAGHUB_ENABLE_TRACE_SYSTEM
#include "UserMarks.h"
DIAGHUB_DECLARE_TRACE
#endif

UserMarks& UserMarks::getInstance() {
    static UserMarks instance;
    return instance;
}

void UserMarks::initialize(const std::string& name) {
#ifdef _WIN32
    USERMARKS_INITIALIZE(std::wstring(name.begin(), name.end()).c_str());
    USERMARKRANGE_INITIALIZE(std::wstring(name.begin(), name.end()).c_str());
#else
    // No-op for non-Windows platforms
#endif
}

void UserMarks::emit(const std::string& message) {
#ifdef _WIN32
    USERMARKS_EMIT(std::wstring(message.begin(), message.end()).c_str());
#else
    // No-op for non-Windows platforms
#endif
}

void UserMarks::startTraceSystem() {
#ifdef _WIN32
    DIAGHUB_START_TRACE_SYSTEM();
#else
    // No-op for non-Windows platforms
#endif
}

void UserMarks::stopTraceSystem() {
#ifdef _WIN32
    DIAGHUB_STOP_TRACE_SYSTEM();
#else
    // No-op for non-Windows platforms
#endif
}

UserMarks::UserMarkRange::UserMarkRange(const std::string& name) {
#ifdef _WIN32
    USERMARKRANGE_START(std::wstring(name.begin(), name.end()).c_str());
#else
    // No-op for non-Windows platforms
#endif
}

UserMarks::UserMarkRange::~UserMarkRange() {
#ifdef _WIN32
    USERMARKRANGE_END();
#else
    // No-op for non-Windows platforms
#endif
}

