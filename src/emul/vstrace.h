#pragma once

#include <string>
#include <memory>

class UserMarks {
public:
    static UserMarks& getInstance();

    // Initialize the user marks system
    void initialize(const std::string& name);

    // Emit a user mark
    void emit(const std::string& message);

    // Start the trace system
    void startTraceSystem();

    // Stop the trace system
    void stopTraceSystem();

    // RAII class for user mark range
    class UserMarkRange {
    public:
        UserMarkRange(const std::string& name);

        virtual ~UserMarkRange();

        // Delete copy constructor and assignment operator
        UserMarkRange(const UserMarkRange&) = delete;
        UserMarkRange& operator=(const UserMarkRange&) = delete;
    };

    std::unique_ptr<UserMarkRange> createUserMarkRange(const std::string& name) {
        return std::make_unique<UserMarkRange>(name);
    }

private:
    UserMarks() = default; // Private constructor
    ~UserMarks() = default; // Private destructor

    // Delete copy constructor and assignment operator
    UserMarks(const UserMarks&) = delete;
    UserMarks& operator=(const UserMarks&) = delete;
};