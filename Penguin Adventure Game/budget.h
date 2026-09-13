#pragma once

#include <chrono>

class FrameBudget {
public:
    FrameBudget(double milliseconds) : limit(milliseconds) {
        start = std::chrono::high_resolution_clock::now();
    }

    bool hasTime() const {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
        return elapsed < limit;
    }

private:
    std::chrono::high_resolution_clock::time_point start;
    double limit;
};