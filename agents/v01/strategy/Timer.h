#pragma once

#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point _startTime;

public:
    Timer();

    [[nodiscard]] double millisecondsSinceStart() const;
};
