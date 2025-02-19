#include "vstrace.h"

#if defined(_WIN32) && defined(DIAGHUB_ENABLE_TRACE_SYSTEM)
#include "UserMarks.h"
DIAGHUB_DECLARE_TRACE
#endif

UserMarks& UserMarks::getInstance() {
    static UserMarks instance;
    return instance;
}


// Active definitions for Windows with DIAGHUB_ENABLE_TRACE_SYSTEM
#if defined(_WIN32) && defined(DIAGHUB_ENABLE_TRACE_SYSTEM)

void UserMarks::initialize(const std::string& name) {
    USERMARKS_INITIALIZE(std::wstring(name.begin(), name.end()).c_str());
    USERMARKRANGE_INITIALIZE(std::wstring(name.begin(), name.end()).c_str());
}

void UserMarks::emit(const std::string& message) {
    USERMARKS_EMIT(std::wstring(message.begin(), message.end()).c_str());
}

void UserMarks::startTraceSystem() {
    DIAGHUB_START_TRACE_SYSTEM();
}

void UserMarks::stopTraceSystem() {
    DIAGHUB_STOP_TRACE_SYSTEM();
}

UserMarks::UserMarkRange::UserMarkRange(const std::string& name) {
    USERMARKRANGE_START(std::wstring(name.begin(), name.end()).c_str());
}

UserMarks::UserMarkRange::~UserMarkRange() {
    USERMARKRANGE_END();
}

#else

// Inactive definitions for non-Windows platforms or without DIAGHUB_ENABLE_TRACE_SYSTEM

void UserMarks::initialize(const std::string& name) {
    // No-op for non-Windows platforms
}

void UserMarks::emit(const std::string& message) {
    // No-op for non-Windows platforms
}

void UserMarks::startTraceSystem() {
    // No-op for non-Windows platforms
}

void UserMarks::stopTraceSystem() {
    // No-op for non-Windows platforms
}

UserMarks::UserMarkRange::UserMarkRange(const std::string& name) {
    // No-op for non-Windows platforms
}

UserMarks::UserMarkRange::~UserMarkRange() {
    // No-op for non-Windows platforms
}

#endif
