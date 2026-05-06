#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace cc {

struct SourceLocation {
    int line = 1;
    int col = 1;
};

[[noreturn]] inline void fail(const std::string& message) {
    throw std::runtime_error(message);
}

inline std::string to_string(const SourceLocation& loc) {
    std::ostringstream oss;
    oss << loc.line << ':' << loc.col;
    return oss.str();
}

class Stopwatch {
public:
    Stopwatch() : start_(clock::now()) {}
    void reset() { start_ = clock::now(); }
    std::int64_t elapsed_ms() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start_).count();
    }

private:
    using clock = std::chrono::steady_clock;
    clock::time_point start_;
};

} // namespace cc
