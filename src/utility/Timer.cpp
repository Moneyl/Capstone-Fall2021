#include "Timer.h"

void Timer::Restart()
{
    _startTime = std::chrono::steady_clock::now();
}

f32 Timer::ElapsedNanoseconds()
{
    _endTime = std::chrono::steady_clock::now();
    return (f32)std::chrono::duration_cast<std::chrono::nanoseconds>(_endTime - _startTime).count();
}

f32 Timer::ElapsedMicroseconds()
{
    _endTime = std::chrono::steady_clock::now();
    return ElapsedNanoseconds() / 1000.0f;
}

f32 Timer::ElapsedMilliseconds()
{
    _endTime = std::chrono::steady_clock::now();
    return ElapsedMicroseconds() / 1000.0f;
}

f32 Timer::ElapsedSeconds()
{
    _endTime = std::chrono::steady_clock::now();
    return ElapsedMilliseconds() / 1000.0f;
}

f32 Timer::ElapsedMinutes()
{
    _endTime = std::chrono::steady_clock::now();
    return ElapsedSeconds() / 60.0f;
}

f32 Timer::ElapsedHours()
{
    _endTime = std::chrono::steady_clock::now();
    return ElapsedMinutes() / 60.0f;
}