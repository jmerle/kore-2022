#include <strategy/Timer.h>

Timer::Timer() : _startTime(std::chrono::high_resolution_clock::now()) {}

double Timer::millisecondsSinceStart() const {
    const auto &duration = std::chrono::high_resolution_clock::now() - _startTime;
    return std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(duration).count();
}
